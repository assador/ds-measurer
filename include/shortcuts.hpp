#pragma once

#include "app_state.hpp"
#include <glib.h>

class ShortcutManager {
  public:
	static void init(const Keybindings& keys);
	static gboolean handle_key(guint keyval, AppState& state);
};
