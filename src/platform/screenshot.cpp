#include "platform/screenshot.hpp"

namespace platform {

namespace kde {
	std::optional<Color> get_pixel_color(GtkWindow*, int, int);
	GdkTexture* get_region_texture(GtkWindow*, int, int, int, int);
}
namespace wayland {
	std::optional<Color> get_pixel_color(GtkWindow*, int, int);
	GdkTexture* get_region_texture(GtkWindow*, int, int, int, int);
}

std::optional<Color> get_pixel_color(GtkWindow* window, int x, int y) {
#if defined(HAS_KDE_SCREENSHOT)
	if (auto res = kde::get_pixel_color(window, x, y)) return res;
#endif
#if defined(HAS_WLR_SCREENCOPY)
	if (auto res = wayland::get_pixel_color(window, x, y)) return res;
#endif
	return std::nullopt;
}

GdkTexture* get_region_texture(GtkWindow* window, int x, int y, int width, int height) {
#if defined(HAS_KDE_SCREENSHOT)
    if (auto res = kde::get_region_texture(window, x, y, width, height)) return res;
#endif
#if defined(HAS_WLR_SCREENCOPY)
    if (auto res = wayland::get_region_texture(window, x, y, width, height)) return res;
#endif
    return nullptr;
}

} // namespace platform
