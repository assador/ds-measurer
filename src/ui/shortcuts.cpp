#include "ui/shortcuts.hpp"
#include <cstdint>
#include <vector>
#include "config/config.hpp"
#include "core/app.hpp"
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

void ShortcutManager::init(const Config& config) {
	g_bindings.clear();

	auto add = [&](const std::string& name, auto&& action) {
		if (auto it = config.keys.find(name); it != config.keys.end()) {
			add_binding(it->second, std::forward<decltype(action)>(action));
		}
	};

	add("quit", [](AppState& state) { state.request_quit(); });
	add("freeze", [](AppState& state) { state.freeze_draft(); });
	add("guide_horizontal", [](AppState& state) { state.add_guide(Orientation::Horizontal); });
	add("guide_vertical", [](AppState& state) { state.add_guide(Orientation::Vertical); });
	add("toggle_mode", [](AppState& state) { ++state.mode; });
	add("select", [](AppState& state) { state.cycle_active(1); });
	add("clear", [](AppState& state) { state.clear_active(); });
	add("clear_last", [](AppState& state) { state.clear_last(); });
	add("clear_all", [](AppState& state) { state.clear_all(); });
	add("relax", [](AppState& state) { state.relax(); });
	add("segment_line", [](AppState& state) { state.toggle_hypot_of_active(); });
	add("fixed_ratio", [](AppState& state, bool is_pressed) { state.set_fixed_ratio(is_pressed); });
	add("from_center", [](AppState& state, bool is_pressed) { state.set_from_center(is_pressed); });

	for (const auto& [kc, ratio] : config.ratios) {
		add_binding(kc, [ratio](AppState& state) {
			state.set_ratio(ratio);
		});
	}
	for (const auto& [kc, theme_name] : config.theme_names) {
		add_binding(kc, [theme_name](AppState& state) {
			state.set_color_scheme(theme_name);
		});
	}
	for (const auto& [kc, rule] : config.guides) {
		add_binding(kc, [kc](AppState& state) {
			state.toggle_grids_of_active(kc);
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
