#include "shortcuts.hpp"
#include <functional>
#include <gtk/gtk.h>
#include <string>
#include <vector>

struct ActionBinding {
	guint keyval{0};
	std::function<void(AppState&)> action;
};

static std::vector<ActionBinding> g_bindings;

void ShortcutManager::init(const Keybindings& keys) {
	g_bindings.clear();

	auto add = [](const std::string& key_name, std::function<void(AppState&)> action) {
		guint kv = gdk_keyval_from_name(key_name.c_str());
		if (kv != GDK_KEY_VoidSymbol) g_bindings.push_back({kv, std::move(action)});
	};

	add(keys.quit, [](AppState&) { g_application_quit(g_application_get_default()); });

	add(keys.freeze, [](AppState& state) {
		if (state.active_measurement) {
			state.frozen_measurements.push_back(
				std::move(*state.active_measurement)
			);
			state.active_measurement.reset();
			state.queue_draw();
		}
	});

	auto set_theme = [&add](const std::string& key, ColorScheme Config::* theme_ptr) {
		add(key, [theme_ptr](AppState& state) {
			state.config.current_theme = state.config.*theme_ptr;
			state.queue_draw();
		});
	};

	set_theme(keys.theme_1, &Config::theme_default);
	set_theme(keys.theme_2, &Config::theme_dark);
	set_theme(keys.theme_3, &Config::theme_white);
	set_theme(keys.theme_4, &Config::theme_black);

	add(keys.clear_last, [](AppState& state) {
		if (!state.frozen_measurements.empty()) {
			state.frozen_measurements.pop_back();
			state.queue_draw();
		}
	});

	auto clear_all = [](AppState& state) {
		state.frozen_measurements.clear();
		state.active_measurement.reset();
		state.queue_draw();
	};

	add(keys.clear, clear_all);
	add(keys.clear_all, clear_all);
}

gboolean ShortcutManager::handle_key(guint keyval, AppState& state) {
	for (const auto& binding : g_bindings) {
		if (binding.keyval == keyval) {
			binding.action(state);
			return TRUE;
		}
	}
	return FALSE;
}
