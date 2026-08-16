#include "platform/screenshot.hpp"

namespace platform {

namespace kde {
	bool region_to_clipboard(GtkWindow*, int, int, int, int);
	std::optional<Color> get_pixel_color(GtkWindow*, int, int);
}
namespace wayland {
	bool region_to_clipboard(GtkWindow*, int, int, int, int);
	std::optional<Color> get_pixel_color(GtkWindow*, int, int);
}

bool region_to_clipboard(GtkWindow* window, int x, int y, int width, int height) {
#if defined(HAS_KDE_SCREENSHOT)
    if (kde::region_to_clipboard(window, x, y, width, height)) return true;
#endif
#if defined(HAS_WLR_SCREENCOPY)
    if (wayland::region_to_clipboard(window, x, y, width, height)) return true;
#endif
    return false;
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

} // namespace platform
