#include "ui/shortcuts.hpp"
#include <functional>
#include <string>
#include <vector>
#include <gtk/gtk.h>

struct ActionBinding {
	guint keyval{0};
	std::function<void(AppState&)> action;
};

static std::vector<ActionBinding> g_bindings;

void ShortcutManager::init(
	const Keybindings& keys,
	const std::vector<std::string>& ratio_keys,
	const std::vector<std::string>& guide_keys
) {
	g_bindings.clear();

	auto add = [&keys](const std::string& kn, std::function<void(AppState&)> action) {
		auto it = keys.find(kn);
		if (it == keys.end() || it->second.empty()) return;
		guint kv = gdk_keyval_from_name(it->second.c_str());
		if (kv != GDK_KEY_VoidSymbol) g_bindings.push_back({kv, std::move(action)});
	};

	add("quit", [](AppState&) { g_application_quit(g_application_get_default()); });

	add("freeze", [](AppState& state) {
		if (state.draft_measurement) {
			state.frozen_measurements.push_back(
				std::make_unique<Measurement>(*state.draft_measurement)
			);
			state.queue_draw();
		}
	});

	add("select_measurement", [](AppState& state) {
		if (state.frozen_measurements.empty()) return;

		size_t current_index = 0;
		bool found = false;

		for (size_t i = 0; i < state.frozen_measurements.size(); ++i) {
			if (state.frozen_measurements[i].get() == state.active_measurement) {
				current_index = i;
				found = true;
				break;
			}
		}
		size_t next_index = found ? (current_index + 1) % state.frozen_measurements.size() : 0;
		state.active_measurement = state.frozen_measurements[next_index].get();
		state.queue_draw();
	});

	add("clear", [](AppState& state) {
		if (!state.active_measurement || state.frozen_measurements.empty()) return;
		for (auto it = state.frozen_measurements.begin(); it != state.frozen_measurements.end(); ++it) {
			if (it->get() == state.active_measurement) {
				state.active_measurement = nullptr;
				state.frozen_measurements.erase(it);
				state.queue_draw();
				return;
			}
		}
	});

	add("clear_last", [](AppState& state) {
		if (!state.frozen_measurements.empty()) {
			if (state.active_measurement == state.frozen_measurements.back().get()) {
				state.active_measurement = nullptr;
			}
			state.frozen_measurements.pop_back();
			state.queue_draw();
		}
	});

	add("clear_all", [](AppState& state) {
		state.frozen_measurements.clear();
		state.active_measurement = nullptr;
		state.draft_measurement.reset();
		state.queue_draw();
	});

	add("relax", [](AppState& state) {
		state.active_measurement = nullptr;
		state.draft_measurement.reset();
		state.queue_draw();
	});

	add("reset_ratio", [](AppState& state) {
		state.reset_ratio();
		if (!state.active_measurement) return;
		auto& m = state.active_measurement;
		m->apply_ratio_with(
			m == state.draft_measurement.get()
				? state.cursor.pos
				: m->end
			,
			state.ratio
		);
		state.queue_draw();
	});

	add("segment_line", [](AppState& state) {
		if (!state.active_measurement) return;
		state.active_measurement->is_hypot_visible = !state.active_measurement->is_hypot_visible;
		state.queue_draw();
	});

	for (const auto& [key, _] : keys) {
		if (key.rfind("theme_", 0) == 0) {
			std::string name = key.substr(6);
			add(key, [name](AppState& state) {
				auto it = state.config.themes.find(name);
				if (it != state.config.themes.end()) {
					state.config.current_theme = &it->second;
					state.queue_draw();
				}
			});
		}
	}
	for (const auto& key : ratio_keys) {
		guint kv = gdk_keyval_from_name(key.c_str());
		if (kv == GDK_KEY_VoidSymbol) continue;
		g_bindings.push_back({
			kv,
			[key](AppState& state) {
				state.toggle_ratio(key);
				if (!state.active_measurement) return;
				auto& m = state.active_measurement;
				m->apply_ratio_with(
					m == state.draft_measurement.get()
						? state.cursor.pos
						: m->end
					,
					state.ratio
				);
				state.queue_draw();
			}
		});
	}
	for (const auto& key : guide_keys) {
		guint kv = gdk_keyval_from_name(key.c_str());
		if (kv == GDK_KEY_VoidSymbol) continue;
		g_bindings.push_back({
			kv,
			[key](AppState& state) {
				if (!state.active_measurement) return;
				state.active_measurement->toggle_grid(key);
				state.queue_draw();
			}
		});
	}
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
