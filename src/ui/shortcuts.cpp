#include "ui/shortcuts.hpp"
#include <cstdint>
#include <vector>
#include "config/config.hpp"
#include "core/app.hpp"
#include "core/guide.hpp"
#include "core/measurement.hpp"
#include "core/utils.hpp"
#include "core/types.hpp"

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

template <typename Container, typename T>
void cycle_active(Container& items, T*& active) {
	if (items.empty()) return;
	size_t current_index = 0;
	bool found = false;
	for (size_t i = 0; i < items.size(); ++i) {
		if (items[i].get() == active) {
			current_index = i;
			found = true;
			break;
		}
	}
	size_t next_index = found ? (current_index + 1) % items.size() : 0;
	active = items[next_index].get();
}

static void redraw_measurement_with_modifiers(Measurement& m, AppState& s) {
	m.apply_modifiers(
		s.lmb.initial_p1,
		s.cursor.pos,
		s.is_from_center,
		s.is_fixed_ratio,
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
			state.measurements.push_back(
				std::make_unique<Measurement>(*state.draft_measurement)
			);
			state.queue_draw();
		}
	});

	add("guide_horizontal", [](AppState& state) {
		state.guides.push_back(
			std::make_unique<Guide>(state.cursor.pos.y, Orientation::Horizontal)
		);
		state.queue_draw();
	});

	add("guide_vertical", [](AppState& state) {
		state.guides.push_back(
			std::make_unique<Guide>(state.cursor.pos.x, Orientation::Vertical)
		);
		state.queue_draw();
	});

	add("toggle_mode", [](AppState& state) {
		++state.mode;
	});

	add("select", [](AppState& state) {
		if (state.mode == Mode::Guides) {
			cycle_active(state.guides, state.active_guide);
		} else {
			cycle_active(state.measurements, state.active_measurement);
		}
		state.queue_draw();
	});

	add("clear", [](AppState& state) {
		auto& active = state.active_measurement;
		if (!active) return;
		auto& frozen = state.measurements;
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
		auto& frozen = state.measurements;
		if (frozen.empty()) return;
		if (state.active_measurement == frozen.back().get()) {
			state.active_measurement = nullptr;
		}
		frozen.pop_back();
		state.queue_draw();
	});

	add("clear_all", [](AppState& state) {
		state.measurements.clear();
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
		active->show_hypot = !active->show_hypot;
		state.queue_draw();
	});

	add("fixed_ratio", [](AppState& state, bool is_pressed) {
		state.is_fixed_ratio = is_pressed;
		auto& active = state.active_measurement;
		if (!active || !state.lmb.is_dragging) return;
		redraw_measurement_with_modifiers(*active, state);
	});

	add("from_center", [](AppState& state, bool is_pressed) {
		state.is_from_center = is_pressed;
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
				!state.is_fixed_ratio
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
				state.color_scheme = &it->second;
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
