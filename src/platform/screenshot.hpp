#pragma once

typedef struct _GtkWindow GtkWindow; // NOLINT

namespace platform {

bool region_to_clipboard(GtkWindow* window, int x, int y, int width, int height);

} // namespace platform
