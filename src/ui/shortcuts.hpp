#pragma once

#include <glib.h>
#include "core/app.hpp"

class ShortcutManager {
public:
	static void init(const Config& config);
	static gboolean handle_key(uint32_t keycode, bool is_pressed, AppState& state);
};
