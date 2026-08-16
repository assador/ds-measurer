#pragma once

#include <optional>
#include "core/types.hpp"

typedef struct _GtkWindow GtkWindow; // NOLINT

namespace platform {

bool region_to_clipboard(GtkWindow* window, int x, int y, int width, int height);
std::optional<Color> get_pixel_color(GtkWindow* window, int x, int y);

} // namespace platform
