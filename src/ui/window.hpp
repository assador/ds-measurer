#pragma once

#include "config/config.hpp"
#include "core/app.hpp"

typedef struct _GtkApplication GtkApplication; // NOLINT
typedef struct _GdkTexture GdkTexture; // NOLINT

void setup_main_window(GtkApplication* app, AppState& state, const Config& config);
bool texture_to_clipboard(GdkTexture* texture);
