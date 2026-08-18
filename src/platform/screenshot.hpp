#pragma once

#include <optional>
#include "core/types.hpp"

typedef struct _GtkWindow GtkWindow; // NOLINT
typedef struct _GdkTexture GdkTexture; // NOLINT

namespace platform {

std::optional<Color> get_pixel_color(GtkWindow* window, int x, int y);
GdkTexture* get_region_texture(GtkWindow* window, int x, int y, int width, int height);

} // namespace platform
