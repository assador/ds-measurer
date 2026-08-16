#include "platform/screenshot.hpp"
#include "core/utils.hpp"

#include <algorithm>
#include <cstring>
#include <string_view>
#include <sys/mman.h>
#include <unistd.h>

#include <gdk/wayland/gdkwayland.h>
#include <gtk/gtk.h>

extern "C" {
#include "wlr-screencopy-v1-protocol.h"
}

namespace platform::wayland {

namespace {

struct ScreencopyFrame {
	std::vector<uint8_t> pixels;
	int width{0};
	int height{0};
	uint32_t stride{0};
	uint32_t format{0};
};

struct ScreencopyData {
	bool done{false};
	bool failed{false};
	int width{0};
	int height{0};
	uint32_t format{0};
	uint32_t stride{0};
	void* pixels{nullptr};
	size_t size{0};
};

static const struct zwlr_screencopy_frame_v1_listener frame_listener = {
	.buffer =
		[](
			void* data,
			struct zwlr_screencopy_frame_v1*,
			uint32_t format,
			uint32_t width,
			uint32_t height,
			uint32_t stride
		) {
			auto* self = static_cast<ScreencopyData*>(data);
			self->format = format;
			self->width = static_cast<int>(width);
			self->height = static_cast<int>(height);
			self->stride = stride;
		}
	,
	.flags =
		[](void*, struct zwlr_screencopy_frame_v1*, uint32_t) {}
	,
	.ready =
		[](
			void* data,
			struct zwlr_screencopy_frame_v1*,
			uint32_t,
			uint32_t,
			uint32_t
		) {
			auto* self = static_cast<ScreencopyData*>(data);
			self->done = true;
		}
	,
	.failed =
		[](
			void* data,
			struct zwlr_screencopy_frame_v1*
		) {
			auto* self = static_cast<ScreencopyData*>(data);
			self->failed = true;
		}
	,
	.damage =
		[](
			void*,
			struct zwlr_screencopy_frame_v1*,
			uint32_t,
			uint32_t,
			uint32_t,
			uint32_t
		) {}
	,
	.linux_dmabuf =
		[](
			void*,
			struct zwlr_screencopy_frame_v1*,
			uint32_t,
			uint32_t,
			uint32_t
		) {}
	,
	.buffer_done =
		[](void*, struct zwlr_screencopy_frame_v1*) {}
	,
};

wl_output* get_wl_output_from_window(
	GdkDisplay* display,
	GtkWindow* window
) {
	GdkSurface* surface = gtk_native_get_surface(GTK_NATIVE(window));
	GdkMonitor* gdk_mon =
		surface ? gdk_display_get_monitor_at_surface(display, surface) : nullptr
	;
	if (!gdk_mon) {
		GListModel* monitors = gdk_display_get_monitors(display);
		if (!monitors || g_list_model_get_n_items(monitors) == 0) return nullptr;
		gdk_mon = GDK_MONITOR(g_list_model_get_item(monitors, 0));
	} else {
		g_object_ref(gdk_mon);
	}
	wl_output* output = gdk_wayland_monitor_get_wl_output(gdk_mon);
	g_object_unref(gdk_mon);
	return output;
}

std::optional<ScreencopyFrame> capture_region_raw(
	GtkWindow* window,
	int x,
	int y,
	int width,
	int height
) {
	if (width <= 0 || height <= 0) return std::nullopt;

	GdkDisplay* display = gdk_display_get_default();
	if (!display) return std::nullopt;

	wl_display* wl_disp = gdk_wayland_display_get_wl_display(display);
	if (!wl_disp) return std::nullopt;

	wl_output* output = get_wl_output_from_window(display, window);
	if (!output) return std::nullopt;

	static zwlr_screencopy_manager_v1* screencopy_mgr = nullptr;
	static wl_shm* wayland_shm = nullptr;

	if (!screencopy_mgr || !wayland_shm) {
		wl_registry* registry = wl_display_get_registry(wl_disp);
		static const wl_registry_listener registry_listener = {
			.global =
				[](
					void*,
					wl_registry* reg,
					uint32_t name,
					const char* interface,
					uint32_t version
				) {
					std::string_view iface(interface);
					if (iface == zwlr_screencopy_manager_v1_interface.name) {
						screencopy_mgr =
							static_cast<zwlr_screencopy_manager_v1*>(
								wl_registry_bind(
									reg,
									name,
									&zwlr_screencopy_manager_v1_interface,
									std::min(version, 3u)
								)
							)
						;
					} else if (iface == wl_shm_interface.name) {
						wayland_shm = static_cast<wl_shm*>(
							wl_registry_bind(reg, name, &wl_shm_interface, 1)
						);
					}
				}
			,
			.global_remove =
				[](
					void*,
					wl_registry*,
					uint32_t
				) {}
			,
		};
		wl_registry_add_listener(registry, &registry_listener, nullptr);
		wl_display_roundtrip(wl_disp);
		wl_registry_destroy(registry);
	}
	if (!screencopy_mgr || !wayland_shm) return std::nullopt;

	zwlr_screencopy_frame_v1* frame =
		zwlr_screencopy_manager_v1_capture_output_region(
			screencopy_mgr, 0, output, x, y, width, height
		)
	;

	ScreencopyData data;
	zwlr_screencopy_frame_v1_add_listener(frame, &frame_listener, &data);

	wl_display_flush(wl_disp);
	while (!data.stride && !data.failed) {
		g_main_context_iteration(nullptr, TRUE);
	}
	if (data.failed || data.stride == 0) {
		zwlr_screencopy_frame_v1_destroy(frame);
		return std::nullopt;
	}

	data.size = data.stride * static_cast<size_t>(data.height);
	int fd = utils::create_shm_file(data.size);
	if (fd < 0) {
		zwlr_screencopy_frame_v1_destroy(frame);
		return std::nullopt;
	}
	data.pixels = mmap(nullptr, data.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data.pixels == MAP_FAILED) {
		close(fd);
		zwlr_screencopy_frame_v1_destroy(frame);
		return std::nullopt;
	}

	wl_shm_pool* pool = wl_shm_create_pool(
		wayland_shm,
		fd,
		static_cast<int32_t>(data.size)
	);
	wl_buffer* buffer = wl_shm_pool_create_buffer(
		pool,
		0,
		data.width,
		data.height,
		static_cast<int32_t>(data.stride),
		data.format
	);
	wl_shm_pool_destroy(pool);
	close(fd);

	zwlr_screencopy_frame_v1_copy(frame, buffer);
	wl_display_flush(wl_disp);

	while (!data.done && !data.failed) {
		g_main_context_iteration(nullptr, TRUE);
	}

	std::optional<ScreencopyFrame> result;
	if (data.done && data.pixels) {
		ScreencopyFrame captured;
		captured.width = data.width;
		captured.height = data.height;
		captured.stride = data.stride;
		captured.format = data.format;
		captured.pixels.resize(data.size);
		std::memcpy(captured.pixels.data(), data.pixels, data.size);
		result = std::move(captured);
	}

	munmap(data.pixels, data.size);
	wl_buffer_destroy(buffer);
	zwlr_screencopy_frame_v1_destroy(frame);

	return result;
}

} // namespace

bool region_to_clipboard(GtkWindow* window, int x, int y, int width, int height) {
	auto frame = capture_region_raw(window, x, y, width, height);
	if (!frame) return false;

	size_t bytes_per_pixel = frame->stride / static_cast<size_t>(frame->width);
	GdkMemoryFormat mem_fmt = GDK_MEMORY_DEFAULT;

	if (bytes_per_pixel == 3) {
		mem_fmt = (frame->format == WL_SHM_FORMAT_RGB888) ? GDK_MEMORY_R8G8B8 : GDK_MEMORY_B8G8R8;
	} else {
		if (frame->format == 0x34325241 || frame->format == 0x34325258) {
			mem_fmt = GDK_MEMORY_R8G8B8A8_PREMULTIPLIED;
		}
		else if (frame->format == WL_SHM_FORMAT_ARGB8888 || frame->format == WL_SHM_FORMAT_XRGB8888) {
			mem_fmt = GDK_MEMORY_B8G8R8A8_PREMULTIPLIED;
		}
	}

	GBytes* bytes = g_bytes_new(frame->pixels.data(), frame->pixels.size());
	GdkTexture* texture = GDK_TEXTURE(gdk_memory_texture_new(
		frame->width,
		frame->height,
		mem_fmt,
		bytes,
		static_cast<gsize>(frame->stride)
	));

	bool success = false;
	if (texture) {
		GdkDisplay* display = gdk_display_get_default();
		GdkClipboard* clipboard = gdk_display_get_clipboard(display);
		gdk_clipboard_set_texture(clipboard, texture);
		g_object_unref(texture);
		success = true;
	}
	g_bytes_unref(bytes);

	return success;
}

std::optional<Color> get_pixel_color(GtkWindow* window, int x, int y) {
	auto frame = capture_region_raw(window, x, y, 1, 1);
	if (!frame || frame->pixels.empty()) return std::nullopt;

	const auto* p = frame->pixels.data();
	Color color;

	if (frame->format == WL_SHM_FORMAT_ARGB8888 || frame->format == WL_SHM_FORMAT_XRGB8888) {
		color.b = p[0] / 255.0;
		color.g = p[1] / 255.0;
		color.r = p[2] / 255.0;
		color.a = (frame->format == WL_SHM_FORMAT_XRGB8888) ? 1.0 : (p[3] / 255.0);
	} else if (frame->format == 0x34325241 || frame->format == 0x34325258) {
		color.r = p[0] / 255.0;
		color.g = p[1] / 255.0;
		color.b = p[2] / 255.0;
		color.a = (frame->format == 0x34325258) ? 1.0 : (p[3] / 255.0);
	} else {
		color.b = p[0] / 255.0;
		color.g = p[1] / 255.0;
		color.r = p[2] / 255.0;
		color.a = 1.0;
	}

	return color;
}

} // namespace platform::wayland
