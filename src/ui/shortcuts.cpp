#include "ui/shortcuts.hpp"
#include <cstdint>
#include <vector>
#include "config/config.hpp"
#include "core/measurement.hpp"

struct ActionBinding {
	uint32_t keycode{0};
	std::function<void(AppState&, bool)> action;
};

static std::vector<ActionBinding> g_bindings;

template <typename Fn>
static void add_binding(uint32_t keycode, Fn&& action) {
	if (keycode == 0) return;
	g_bindings.push_back({
		keycode,
		[action = std::forward<Fn>(action)](AppState& state, bool is_pressed) {
			if constexpr (std::is_invocable_v<Fn, AppState&, bool>) {
				action(state, is_pressed);
			} else if (is_pressed) {
				action(state);
			}
		}
	});
}

static void redraw_measurement_with_modifiers(Measurement& m, AppState& s) {
	m.apply_modifiers(
		s.lmb.initial_p1,
		s.cursor.pos,
		s.is_pressed_from_center,
		s.is_pressed_fixed_ratio,
		s.ratio
	);
	s.queue_draw();
}

void ShortcutManager::init(const Config& config) {
	g_bindings.clear();

	auto add = [&](const std::string& name, auto&& action) {
		if (auto it = config.keys.find(name); it != config.keys.end()) {
			add_binding(it->second, std::forward<decltype(action)>(action));
		}
	};

	add("quit", [](AppState& state) {
		state.request_quit();
	});

	add("freeze", [](AppState& state) {
		if (state.draft_measurement) {
			state.frozen_measurements.push_back(
				std::make_unique<Measurement>(*state.draft_measurement)
			);
			state.queue_draw();
		}
	});

	add("select_measurement", [](AppState& state) {
		auto& frozen = state.frozen_measurements;
		if (frozen.empty()) return;
		size_t current_index = 0;
		bool found = false;
		for (size_t i = 0; i < frozen.size(); ++i) {
			if (frozen[i].get() == state.active_measurement) {
				current_index = i;
				found = true;
				break;
			}
		}
		size_t next_index = found ? (current_index + 1) % frozen.size() : 0;
		state.active_measurement = frozen[next_index].get();
		state.queue_draw();
	});

	add("clear", [](AppState& state) {
		auto& active = state.active_measurement;
		if (!active) return;
		auto& frozen = state.frozen_measurements;
		for (auto it = frozen.begin(); it != frozen.end(); ++it) {
			if (it->get() == active) {
				active = nullptr;
				frozen.erase(it);
				state.queue_draw();
				return;
			}
		}
	});

	add("clear_last", [](AppState& state) {
		auto& frozen = state.frozen_measurements;
		if (frozen.empty()) return;
		if (state.active_measurement == frozen.back().get()) {
			state.active_measurement = nullptr;
		}
		frozen.pop_back();
		state.queue_draw();
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

	add("segment_line", [](AppState& state) {
		auto& active = state.active_measurement;
		if (!active) return;
		active->is_hypot_visible = !active->is_hypot_visible;
		state.queue_draw();
	});

	add("fixed_ratio", [](AppState& state, bool is_pressed) {
		state.is_pressed_fixed_ratio = is_pressed;
		auto& active = state.active_measurement;
		if (!active || !state.lmb.is_dragging) return;
		redraw_measurement_with_modifiers(*active, state);
	});

	add("from_center", [](AppState& state, bool is_pressed) {
		state.is_pressed_from_center = is_pressed;
		auto& active = state.active_measurement;
		if (!active || !state.lmb.is_dragging) return;
		redraw_measurement_with_modifiers(*active, state);
	});

	for (const auto& [kc, ratio] : config.ratios) {
		add_binding(kc, [ratio](AppState& state) {
			state.ratio = ratio;
			if (
				!state.active_measurement ||
				!state.lmb.is_dragging ||
				!state.is_pressed_fixed_ratio
			) return;
			redraw_measurement_with_modifiers(*state.active_measurement, state);
		});
	}
	for (const auto& [kc, rule] : config.guides) {
		add_binding(kc, [kc](AppState& state) {
			if (!state.active_measurement) return;
			state.active_measurement->toggle_grid(kc);
			state.queue_draw();
		});
	}
	for (const auto& [kc, theme_name] : config.theme_names) {
		add_binding(kc, [theme_name](AppState& state) {
			if (
				auto it = state.config.color_schemes.find(theme_name);
				it != state.config.color_schemes.end()
			) {
				state.config.current_color_scheme = &it->second;
				state.queue_draw();
			}
		});
	}
}

gboolean ShortcutManager::handle_key(uint32_t keycode, bool is_pressed, AppState& state) {
	for (const auto& binding : g_bindings) {
		if (binding.keycode == keycode) {
			binding.action(state, is_pressed);
			return TRUE;
		}
	}
	return FALSE;
}
