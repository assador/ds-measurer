#pragma once

#include "core/app_state.hpp"
#include <glib.h>

class ShortcutManager {
public:
	static void init(
		const Keybindings& keys,
		const std::vector<std::string>& guide_keys
	);
	static gboolean handle_key(guint keyval, AppState& state);
};
