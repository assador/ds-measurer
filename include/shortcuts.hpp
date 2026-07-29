#pragma once

#include "app_state.hpp"
#include <glib.h>

typedef struct _GtkApplication GtkApplication;
typedef struct _GtkWidget GtkWidget;

class ShortcutManager {
public:
	static void init(const Keybindings& keys);
	static gboolean handle_key(guint keyval, GtkWidget* widget, GtkApplication* app, AppState& state);
};
