#pragma once

#include "config.hpp"
#include "app_state.hpp"

typedef struct _GtkApplication GtkApplication;

void setup_main_window(GtkApplication* app, AppState& state, const Config& config);
