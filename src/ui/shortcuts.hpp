#pragma once

#include <glib.h>
#include "core/app_state.hpp"

class ShortcutManager {
public:
	static void init(
		const Keybindings& keys,
		const std::vector<std::string>& ratio_keys,
		const std::vector<std::string>& guide_keys
	);
	static gboolean handle_key(guint keyval, AppState& state);
};
