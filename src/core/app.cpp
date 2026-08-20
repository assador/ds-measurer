#include "core/app.hpp"
#include <gtk/gtk.h>
#include "config/config.hpp"
#include "core/shobzaebis.hpp"
#include "core/types.hpp"
#include "ui/shortcuts.hpp"
#include "ui/window.hpp"

void AppState::request_quit() {
	g_application_quit(g_application_get_default());
}

void AppState::apply_from_config() {
	if (auto it = config.color_schemes.find("default"); it != config.color_schemes.end()) {
		color_scheme = &it->second;
	} else if (!config.color_schemes.empty()) {
		color_scheme = &config.color_schemes.begin()->second;
	}
}

void AppState::set_color_scheme(const std::string& name) {
	if (
		auto it = config.color_schemes.find(name);
		it != config.color_schemes.end()
	) {
		color_scheme = &it->second;
		redraw();
	}
}

void AppState::add_guide(Orientation orient) {
	guides.push_back(std::make_unique<Guide>(
		orient == Orientation::Vertical ? cursor.pos.x : cursor.pos.y,
		orient
	));
	redraw();
}

void AppState::clear_active() {
	if (mode == Mode::ColorPicks) {
		clear_container_active(color_picks, active_color_pick);
	} else if (mode == Mode::Guides) {
		clear_container_active(guides, active_guide);
	} else {
		clear_container_active(measurements, active_measurement);
	}
	redraw();
}

void AppState::clear_all() {
	if (mode == Mode::ColorPicks) {
		color_picks.clear();
		active_color_pick = nullptr;
	} else if (mode == Mode::Guides) {
		guides.clear();
		active_guide = nullptr;
	} else {
		measurements.clear();
		active_measurement = nullptr;
		draft_measurement.reset();
	}
	redraw();
}

void AppState::clear_last() {
	if (mode == Mode::ColorPicks) {
		clear_container_last(color_picks, active_color_pick);
	} else if (mode == Mode::Guides) {
		clear_container_last(guides, active_guide);
	} else {
		clear_container_last(measurements, active_measurement);
	}
	redraw();
}

void AppState::cycle_active(int step) {
	if (mode == Mode::ColorPicks) {
		cycle_container_active(color_picks, active_color_pick, step);
	} else if (mode == Mode::Guides) {
		cycle_container_active(guides, active_guide, step);
	} else {
		cycle_container_active(measurements, active_measurement, step);
	}
	redraw();
}

void AppState::relax() {
	if (mode == Mode::ColorPicks) {
		active_color_pick = nullptr;
	} else if (mode == Mode::Guides) {
		active_guide = nullptr;
	} else {
		active_measurement = nullptr;
		draft_measurement.reset();
	}
	redraw();
}

void AppState::copy_values(Measurement* m) {
	if (!m) return;
	text_to_clipboard(m->values_string(config.copy_format));
}

void AppState::copy_active_color() {
	if (active_color_pick) text_to_clipboard(active_color_pick->fmt_str);
}

void AppState::freeze_draft() {
	if (draft_measurement) {
		measurements.push_back(std::make_unique<Measurement>(*draft_measurement));
		redraw();
	}
}

void AppState::pick_color(Cursor& c) {
	if (auto color = get_pixel_color(c.pos.x, c.pos.y)) {
		if (config.color_pick.target != ColorPickTarget::Clipboard) {
			color_picks.push_back(std::make_unique<ColorPick>(c.pos, color));
			redraw();
		}
		if (config.color_pick.target != ColorPickTarget::Stack) {
			color_to_clipboard(*color);
		}
	}
}

// SEC Actions on active objects and modifiers

[[nodiscard]] Point AppState::get_snapped_pos(Point initial) const {
	if (!snap_to_guides || guides.empty()) return initial;

	int dist = config.snap_distance;
	if (dist <= 0) return initial;
	Point snapped = initial;

	for (const auto& g : guides) {
		if (g->orientation == Orientation::Horizontal) {
			if (std::abs(initial.y - g->coord) <= dist) snapped.y = g->coord;
		} else if (g->orientation == Orientation::Vertical) {
			if (std::abs(initial.x - g->coord) <= dist) snapped.x = g->coord;
		}
	}
	return snapped;
}

void AppState::redraw_measurement_with_modifiers(Measurement& m) {
	m.apply_modifiers(
		lmb.initial_p1,
		cursor.pos,
		is_from_center,
		is_fixed_ratio,
		ratio
	);
	redraw();
}

void AppState::set_fixed_ratio(bool is_pressed) {
	is_fixed_ratio = is_pressed;
	if (active_measurement && lmb.is_dragging) {
		redraw_measurement_with_modifiers(*active_measurement);
	}
}

void AppState::set_from_center(bool is_pressed) {
	is_from_center = is_pressed;
	if (active_measurement && lmb.is_dragging) {
		redraw_measurement_with_modifiers(*active_measurement);
	}
}

void AppState::set_ratio(double r) {
	ratio = r;
	if (!active_measurement || !lmb.is_dragging || !is_fixed_ratio) return;
	redraw_measurement_with_modifiers(*active_measurement);
}

void AppState::snap_active_translation() {
	auto& m = active_measurement;
	auto& p = active_color_pick;
	if (!snap_to_guides || guides.empty() || !m && !p) return;

	int snap_dx = 0;
	int snap_dy = 0;

	if (m && (mode == Mode::Measurements || m == draft_measurement.get())) {
		Point snapped_start = get_snapped_pos(m->start);
		if (snapped_start.x != m->start.x) snap_dx = snapped_start.x - m->start.x;
		if (snapped_start.y != m->start.y) snap_dy = snapped_start.y - m->start.y;

		Point snapped_end = get_snapped_pos(m->end);
		if (snap_dx == 0 && snapped_end.x != m->end.x) snap_dx = snapped_end.x - m->end.x;
		if (snap_dy == 0 && snapped_end.y != m->end.y) snap_dy = snapped_end.y - m->end.y;

		if (snap_dx != 0 || snap_dy != 0)  m->move_by(snap_dx, snap_dy);
	}
	else if (p && mode == Mode::ColorPicks) {
		Point snapped = get_snapped_pos(p->pos);
		if (snapped.x != p->pos.x) snap_dx = snapped.x - p->pos.x;
		if (snapped.y != p->pos.y) snap_dy = snapped.y - p->pos.y;

		if (snap_dx != 0 || snap_dy != 0)  p->move_by(snap_dx, snap_dy);
	}
}

void AppState::toggle_grids_of_active(uint32_t kc) {
	if (!active_measurement) return;
	active_measurement->toggle_grid(kc);
	redraw();
}

void AppState::toggle_diagonal_of_active() {
	if (!active_measurement) return;
	active_measurement->show_diagonal = !active_measurement->show_diagonal;
	redraw();
}

int run_application(int argc, char** argv, AppState& state, Config& config) {
	GtkApplication* app = gtk_application_new(
		"org.assador.ds.Measurer",
		G_APPLICATION_DEFAULT_FLAGS
	);
	g_signal_connect(
		app,
		"activate",
		G_CALLBACK(+[](GtkApplication* app, gpointer user_data) {
			auto* state = static_cast<AppState*>(user_data);
			state->config.load_from_file(resolve_config_path());
			state->apply_from_config();
			ShortcutManager::init(state->config);
			setup_main_window(app, *state, state->config);
		}),
		&state
	);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	shobzaebis::do_everything();
	g_object_unref(app);
	return status;
}
