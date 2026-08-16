#include "platform/screenshot.hpp"

#if defined(HAS_KDE_SCREENSHOT)
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include <gtk/gtk.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

namespace platform::kde {

namespace {

struct KWinFrame {
	std::vector<uint8_t> pixels;
	uint32_t width{0};
	uint32_t height{0};
	uint32_t stride{0};
};

std::optional<KWinFrame> capture_region_raw(int x, int y, int width, int height) {
	if (width <= 0 || height <= 0) return std::nullopt;

	const auto u_width = static_cast<uint32_t>(width);
	const auto u_height = static_cast<uint32_t>(height);
	const uint32_t stride = u_width * 4;
	const size_t buffer_size = static_cast<size_t>(stride) * u_height;

	int mem_fd = memfd_create("kwin_screenshot", MFD_CLOEXEC);
	if (mem_fd < 0) return std::nullopt;

	if (ftruncate(mem_fd, static_cast<off_t>(buffer_size)) < 0) {
		close(mem_fd);
		return std::nullopt;
	}

	GError* error = nullptr;
	GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
	if (!conn) {
		if (error) g_error_free(error);
		close(mem_fd);
		return std::nullopt;
	}

	GUnixFDList* fd_list = g_unix_fd_list_new();
	int fd_index = g_unix_fd_list_append(fd_list, mem_fd, &error);
	if (fd_index < 0) {
		if (error) g_error_free(error);
		g_object_unref(fd_list);
		g_object_unref(conn);
		close(mem_fd);
		return std::nullopt;
	}

	GVariantBuilder options_builder;
	g_variant_builder_init(&options_builder, G_VARIANT_TYPE("a{sv}"));

	GVariant* result =
		g_dbus_connection_call_with_unix_fd_list_sync(
			conn,
			"org.kde.KWin.ScreenShot2",
			"/org/kde/KWin/ScreenShot2",
			"org.kde.KWin.ScreenShot2",
			"CaptureArea",
			g_variant_new(
				"(iiuua{sv}h)",
				x,
				y,
				u_width,
				u_height,
				&options_builder,
				fd_index
			),
			G_VARIANT_TYPE("(a{sv})"),
			G_DBUS_CALL_FLAGS_NONE,
			5000,
			fd_list,
			nullptr,
			nullptr,
			&error
		)
	;

	g_object_unref(fd_list);

	if (!result) {
		if (error) {
			g_warning("KWin ScreenShot2 D-Bus error: %s", error->message);
			g_error_free(error);
		}
		g_object_unref(conn);
		close(mem_fd);
		return std::nullopt;
	}

	g_variant_unref(result);
	g_object_unref(conn);

	std::optional<KWinFrame> frame;
	void* ptr = mmap(nullptr, buffer_size, PROT_READ, MAP_SHARED, mem_fd, 0);
	if (ptr != MAP_FAILED) {
		KWinFrame captured;
		captured.width = u_width;
		captured.height = u_height;
		captured.stride = stride;
		captured.pixels.resize(buffer_size);
		std::memcpy(captured.pixels.data(), ptr, buffer_size);
		munmap(ptr, buffer_size);
		frame = std::move(captured);
	}

	close(mem_fd);
	return frame;
}

} // namespace

bool region_to_clipboard(GtkWindow* /*window*/, int x, int y, int width, int height) {
	auto frame = capture_region_raw(x, y, width, height);
	if (!frame) return false;

	GBytes* bytes = g_bytes_new(frame->pixels.data(), frame->pixels.size());
	GdkTexture* texture = GDK_TEXTURE(gdk_memory_texture_new(
		static_cast<int>(frame->width),
		static_cast<int>(frame->height),
		GDK_MEMORY_B8G8R8A8_PREMULTIPLIED,
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

std::optional<Color> get_pixel_color(GtkWindow* /*window*/, int x, int y) {
	auto frame = capture_region_raw(x, y, 1, 1);
	if (!frame || frame->pixels.size() < 4) return std::nullopt;

	const uint8_t* p = frame->pixels.data();

	Color color;
	color.b = p[0] / 255.0;
	color.g = p[1] / 255.0;
	color.r = p[2] / 255.0;
	color.a = 1.0;

	return color;
}

} // namespace platform::kde

#else

namespace platform::kde {

bool region_to_clipboard(GtkWindow*, int, int, int, int) { return false; }
std::optional<Color> get_pixel_color(GtkWindow*, int, int) { return std::nullopt; }

} // namespace platform::kde

#endif
