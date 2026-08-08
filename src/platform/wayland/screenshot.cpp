#include "platform/screenshot.hpp"
#include "core/utils.hpp"

#include <algorithm>
#include <string_view>
#include <sys/mman.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <gdk/wayland/gdkwayland.h>

extern "C" {
#include "wlr-screencopy-v1-protocol.h"
}

namespace platform::wayland {

namespace {

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
	.buffer = [](void* data, struct zwlr_screencopy_frame_v1*, uint32_t format, uint32_t width, uint32_t height, uint32_t stride) {
		auto* self = static_cast<ScreencopyData*>(data);
		self->format = format;
		self->width = static_cast<int>(width);
		self->height = static_cast<int>(height);
		self->stride = stride;
	},
	.flags = [](void*, struct zwlr_screencopy_frame_v1*, uint32_t) {},
	.ready = [](void* data, struct zwlr_screencopy_frame_v1*, uint32_t, uint32_t, uint32_t) {
		auto* self = static_cast<ScreencopyData*>(data);
		self->done = true;
	},
	.failed = [](void* data, struct zwlr_screencopy_frame_v1*) {
		auto* self = static_cast<ScreencopyData*>(data);
		self->failed = true;
	},
	.damage = [](void*, struct zwlr_screencopy_frame_v1*, uint32_t, uint32_t, uint32_t, uint32_t) {},
	.linux_dmabuf = [](void*, struct zwlr_screencopy_frame_v1*, uint32_t, uint32_t, uint32_t) {},
	.buffer_done = [](void*, struct zwlr_screencopy_frame_v1*) {},
};

} // namespace

bool region_to_clipboard(GtkWindow* window, int x, int y, int width, int height) {
	if (width <= 0 || height <= 0) return false;

	GdkDisplay* display = gdk_display_get_default();
	if (!display) return false;

	wl_display* wl_disp = gdk_wayland_display_get_wl_display(display);
	if (!wl_disp) return false;

	static zwlr_screencopy_manager_v1* screencopy_mgr = nullptr;
	static wl_shm* wayland_shm = nullptr;

	if (!screencopy_mgr || !wayland_shm) {
		wl_registry* registry = wl_display_get_registry(wl_disp);
		static const wl_registry_listener registry_listener = {
			.global = [](void*, wl_registry* reg, uint32_t name, const char* interface, uint32_t version) {
				std::string_view iface(interface);
				if (iface == zwlr_screencopy_manager_v1_interface.name) {
					screencopy_mgr = static_cast<zwlr_screencopy_manager_v1*>(
						wl_registry_bind(reg, name, &zwlr_screencopy_manager_v1_interface, std::min(version, 3u))
					);
				} else if (iface == wl_shm_interface.name) {
					wayland_shm = static_cast<wl_shm*>(
						wl_registry_bind(reg, name, &wl_shm_interface, 1)
					);
				}
			},
			.global_remove = [](void*, wl_registry*, uint32_t) {}
		};
		wl_registry_add_listener(registry, &registry_listener, nullptr);
		wl_display_roundtrip(wl_disp);
		wl_registry_destroy(registry);
	}

	if (!screencopy_mgr || !wayland_shm) {
		g_warning("wlr-screencopy protocol or wl_shm is not supported!");
		return false;
	}

	gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
	while (g_main_context_pending(nullptr)) {
		g_main_context_iteration(nullptr, FALSE);
	}

	zwlr_screencopy_frame_v1* frame = zwlr_screencopy_manager_v1_capture_output_region(
		screencopy_mgr, 0, nullptr, x, y, width, height
	);

	ScreencopyData data;
	zwlr_screencopy_frame_v1_add_listener(frame, &frame_listener, &data);

	while (!data.width && !data.failed) {
		wl_display_dispatch(wl_disp);
	}

	if (data.failed) {
		zwlr_screencopy_frame_v1_destroy(frame);
		gtk_widget_set_visible(GTK_WIDGET(window), TRUE);
		return false;
	}

	data.size = data.stride * static_cast<size_t>(data.height);
	int fd = utils::create_shm_file(data.size);
	if (fd < 0) {
		zwlr_screencopy_frame_v1_destroy(frame);
		gtk_widget_set_visible(GTK_WIDGET(window), TRUE);
		return false;
	}

	data.pixels = mmap(nullptr, data.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	wl_shm_pool* pool = wl_shm_create_pool(wayland_shm, fd, static_cast<int32_t>(data.size));
	wl_buffer* buffer = wl_shm_pool_create_buffer(
		pool, 0, data.width, data.height, static_cast<int32_t>(data.stride), data.format
	);
	wl_shm_pool_destroy(pool);
	close(fd);

	zwlr_screencopy_frame_v1_copy(frame, buffer);

	while (!data.done && !data.failed) {
		wl_display_dispatch(wl_disp);
	}

	gtk_widget_set_visible(GTK_WIDGET(window), TRUE);

	bool success = false;
	if (data.done && data.pixels) {
		GBytes* bytes = g_bytes_new_with_free_func(
			data.pixels, data.size, [](void* ptr) { munmap(ptr, 0); }, nullptr
		);

		GdkTexture* texture = gdk_memory_texture_new(
			data.width, data.height,
			GDK_MEMORY_B8G8R8A8_PREMULTIPLIED,
			bytes, data.stride
		);
		g_bytes_unref(bytes);

		GdkClipboard* clipboard = gdk_display_get_clipboard(display);
		gdk_clipboard_set_texture(clipboard, texture);
		g_object_unref(texture);
		success = true;
	}

	wl_buffer_destroy(buffer);
	zwlr_screencopy_frame_v1_destroy(frame);
	return success;
}

} // namespace platform::wayland
