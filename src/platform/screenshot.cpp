#include "platform/screenshot.hpp"

namespace platform {

namespace kde { bool region_to_clipboard(GtkWindow*, int, int, int, int); }
namespace wayland { bool region_to_clipboard(GtkWindow*, int, int, int, int); }

bool region_to_clipboard(GtkWindow* window, int x, int y, int width, int height) {
#if defined(HAS_KDE_SCREENSHOT)
    if (kde::region_to_clipboard(window, x, y, width, height)) return true;
#endif
#if defined(HAS_WLR_SCREENCOPY)
    if (wayland::region_to_clipboard(window, x, y, width, height)) return true;
#endif
    return false;
}

} // namespace platform
