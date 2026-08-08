#include "platform/screenshot.hpp"

#if defined(HAS_KDE_SCREENSHOT)
#include <cstddef>
#include <cstdint>
#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include <gtk/gtk.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

namespace platform::kde {

static void copy_bgra_to_rgba(
	const uint8_t* src,
	uint8_t* dst,
	uint32_t width,
	uint32_t height,
	uint32_t stride
) {
	for (uint32_t y = 0; y < height; ++y) {
		const auto* src_row = reinterpret_cast<const uint32_t*>(
			src + static_cast<size_t>(y) * stride
		);
		auto* dst_row = reinterpret_cast<uint32_t*>(
			dst + static_cast<size_t>(y) * width * 4
		);
		for (uint32_t x = 0; x < width; ++x) {
			uint32_t pixel = src_row[x];
			uint8_t b = (pixel >> 0) & 0xFF;
			uint8_t g = (pixel >> 8) & 0xFF;
			uint8_t r = (pixel >> 16) & 0xFF;
			uint8_t a = (pixel >> 24) & 0xFF;
			dst_row[x] = (a << 24) | (b << 16) | (g << 8) | r;
		}
	}
}

bool region_to_clipboard(
	GtkWindow* window,
	int x,
	int y,
	int width,
	int height
) {
	if (width <= 0 || height <= 0) return false;

	auto u_width = static_cast<uint32_t>(width);
	auto u_height = static_cast<uint32_t>(height);
	uint32_t stride = u_width * 4;
	size_t buffer_size = static_cast<size_t>(stride) * u_height;

	int mem_fd = memfd_create("kwin_screenshot", MFD_CLOEXEC);
	if (mem_fd < 0) return false;

	if (ftruncate(mem_fd, static_cast<off_t>(buffer_size)) < 0) {
		close(mem_fd);
		return false;
	}

	GError* error = nullptr;
	GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
	if (!conn) {
		if (error) g_error_free(error);
		close(mem_fd);
		return false;
	}

	gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
	while (g_main_context_pending(nullptr)) {
		g_main_context_iteration(nullptr, FALSE);
	}

	GUnixFDList* fd_list = g_unix_fd_list_new();
	int fd_index = g_unix_fd_list_append(fd_list, mem_fd, &error);
	if (fd_index < 0) {
		if (error) g_error_free(error);
		g_object_unref(fd_list);
		g_object_unref(conn);
		close(mem_fd);
		gtk_widget_set_visible(GTK_WIDGET(window), TRUE);
		return false;
	}

	GVariantBuilder options_builder;
	g_variant_builder_init(&options_builder, G_VARIANT_TYPE("a{sv}"));

	GVariant* result = g_dbus_connection_call_with_unix_fd_list_sync(
		conn,
		"org.kde.KWin.ScreenShot2",
		"/org/kde/KWin/ScreenShot2",
		"org.kde.KWin.ScreenShot2",
		"CaptureArea",
		g_variant_new("(iiuua{sv}h)", x, y, u_width, u_height, &options_builder, fd_index),
		G_VARIANT_TYPE("(a{sv})"),
		G_DBUS_CALL_FLAGS_NONE,
		5000,
		fd_list,
		nullptr,
		nullptr,
		&error
	);

	gtk_widget_set_visible(GTK_WIDGET(window), TRUE);
	g_object_unref(fd_list);

	if (!result) {
		if (error) {
			g_warning("KWin ScreenShot2 D-Bus error: %s", error->message);
			g_error_free(error);
		}
		g_object_unref(conn);
		close(mem_fd);
		return false;
	}

	g_variant_unref(result);
	g_object_unref(conn);

	void* ptr = mmap(nullptr, buffer_size, PROT_READ, MAP_SHARED, mem_fd, 0);
	if (ptr != MAP_FAILED) {
		std::vector<uint8_t> rgba_data(static_cast<size_t>(u_width) * u_height * 4);
		copy_bgra_to_rgba(
			static_cast<const uint8_t*>(ptr), rgba_data.data(), u_width,
			u_height, stride
		);
		GBytes* rgba_bytes = g_bytes_new(rgba_data.data(), rgba_data.size());
		GdkTexture* texture = GDK_TEXTURE(gdk_memory_texture_new(
			static_cast<int>(u_width),
			static_cast<int>(u_height),
			GDK_MEMORY_R8G8B8A8,
			rgba_bytes,
			static_cast<gsize>(u_width) * 4
		));
		if (texture) {
			GdkDisplay* display = gdk_display_get_default();
			GdkClipboard* clipboard = gdk_display_get_clipboard(display);
			gdk_clipboard_set_texture(clipboard, texture);
			g_object_unref(texture);
		}
		g_bytes_unref(rgba_bytes);
		munmap(ptr, buffer_size);
	}
	close(mem_fd);
	return true;
}

} // namespace platform::kde

#else

namespace platform::kde {
bool region_to_clipboard(GtkWindow*, int, int, int, int) { return false; }
} // namespace platform::kde

#endif
