#pragma once

#include "config/config.hpp"
#include "core/app_state.hpp"

typedef struct _GtkApplication GtkApplication;

void setup_main_window(GtkApplication* app, AppState& state, const Config& config);
