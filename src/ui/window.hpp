#pragma once

#include "core/app_state.hpp"
#include "config/config.hpp"

typedef struct _GtkApplication GtkApplication;

void setup_main_window(GtkApplication* app, AppState& state, const Config& config);
