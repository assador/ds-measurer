#pragma once

#include <functional>
#include <memory>
#include <vector>
#include "config/config.hpp"
#include "core/cursor.hpp"
#include "core/guide.hpp"
#include "core/measurement.hpp"
#include "core/types.hpp"

struct MouseState {
	bool is_dragging{false};
	Point click_pos;
	Point initial_p1;
	Point initial_p2;
};

struct AppState {
	Config& config;
	Cursor cursor;
	MouseState lmb;
	MouseState rmb;

	Mode mode{Mode::Measurements};

	double ratio{1.1};
	bool is_fixed_ratio{false};
	bool is_from_center{false};

	bool show_guides{true};
	bool snap_to_guides{true};
	
	std::vector<std::unique_ptr<Guide>> guides;
	std::vector<std::unique_ptr<Measurement>> measurements;
	std::unique_ptr<Measurement> draft_measurement;
	Guide* active_guide = nullptr;
	Measurement* active_measurement = nullptr;

	ColorScheme* color_scheme = nullptr;

	explicit AppState(Config& cfg) : config(cfg) {}

	void set_color_scheme(const std::string& name) {
		if (
			auto it = config.color_schemes.find(name);
			it != config.color_schemes.end()
		) {
			color_scheme = &it->second;
			queue_draw();
		}
	}

	void freeze_draft() {
		if (draft_measurement) {
			measurements.push_back(std::make_unique<Measurement>(*draft_measurement));
			queue_draw();
		}
	}

	void add_guide(Orientation orient) {
		guides.push_back(std::make_unique<Guide>(
			orient == Orientation::Vertical ? cursor.pos.x : cursor.pos.y,
			orient
		));
		queue_draw();
	}

	void cycle_active(int step = 1) {
		if (mode == Mode::Guides) {
			cycle_container_active(guides, active_guide, step);
		} else {
			cycle_container_active(measurements, active_measurement, step);
		}
		queue_draw();
	}

	void clear_active() {
		if (mode == Mode::Guides) {
			clear_container_active(guides, active_guide);
		} else {
			clear_container_active(measurements, active_measurement);
		}
		queue_draw();
	}

	void clear_last() {
		if (mode == Mode::Guides) {
			clear_container_last(guides, active_guide);
		} else {
			clear_container_last(measurements, active_measurement);
		}
		queue_draw();
	}

	void clear_all() {
		if (mode == Mode::Guides) {
			guides.clear();
			active_guide = nullptr;
		} else {
			measurements.clear();
			active_measurement = nullptr;
			draft_measurement.reset();
		}
		queue_draw();
	}

	void relax() {
		if (mode == Mode::Guides) {
			active_guide = nullptr;
		} else {
			active_measurement = nullptr;
			draft_measurement.reset();
		}
		queue_draw();
	}

// SEC Actions on active objects and modifiers

	void redraw_measurement_with_modifiers(Measurement& m) {
		m.apply_modifiers(
			lmb.initial_p1,
			cursor.pos,
			is_from_center,
			is_fixed_ratio,
			ratio
		);
		queue_draw();
	}

	void set_ratio(double r) {
		ratio = r;
		if (!active_measurement || !lmb.is_dragging || !is_fixed_ratio) return;
		redraw_measurement_with_modifiers(*active_measurement);
	}

	void set_fixed_ratio(bool is_pressed) {
		is_fixed_ratio = is_pressed;
		if (active_measurement && lmb.is_dragging) {
			redraw_measurement_with_modifiers(*active_measurement);
		}
	}

	void set_from_center(bool is_pressed) {
		is_from_center = is_pressed;
		if (active_measurement && lmb.is_dragging) {
			redraw_measurement_with_modifiers(*active_measurement);
		}
	}

	void toggle_hypot_of_active() {
		if (!active_measurement) return;
		active_measurement->show_hypot = !active_measurement->show_hypot;
		queue_draw();
	}

	void toggle_grids_of_active(uint32_t kc) {
		if (!active_measurement) return;
		active_measurement->toggle_grid(kc);
		queue_draw();
	}

	[[nodiscard]] Point get_snapped_pos(Point initial) const {
		if (!snap_to_guides || guides.empty()) return initial;

		int dist = config.snap_distance;
		if (dist <= 0) return initial;
		Point snapped = initial;

		for (const auto& g : guides) {
			if (g->orientation == Orientation::Horizontal) {
				if (std::abs(initial.y - g->position) <= dist) snapped.y = g->position;
			} else if (g->orientation == Orientation::Vertical) {
				if (std::abs(initial.x - g->position) <= dist) snapped.x = g->position;
			}
		}
		return snapped;
	}

	void snap_active_translation() {
		if (!snap_to_guides || guides.empty() || !active_measurement) return;

		auto& m = *active_measurement;
		int snap_dx = 0;
		int snap_dy = 0;

		Point snapped_start = get_snapped_pos(m.start);
		if (snapped_start.x != m.start.x) snap_dx = snapped_start.x - m.start.x;
		if (snapped_start.y != m.start.y) snap_dy = snapped_start.y - m.start.y;

		Point snapped_end = get_snapped_pos(m.end);
		if (snap_dx == 0 && snapped_end.x != m.end.x) snap_dx = snapped_end.x - m.end.x;
		if (snap_dy == 0 && snapped_end.y != m.end.y) snap_dy = snapped_end.y - m.end.y;

		if (snap_dx != 0 || snap_dy != 0)  m.move_by(snap_dx, snap_dy);
	}

	std::function<void()> request_draw;
	void queue_draw() const {
		if (request_draw) request_draw();
	}

	void apply_from_config() {
		if (auto it = config.color_schemes.find("default"); it != config.color_schemes.end()) {
			color_scheme = &it->second;
		} else if (!config.color_schemes.empty()) {
			color_scheme = &config.color_schemes.begin()->second;
		}
	};

	void request_quit();

private:

	template <typename Container, typename T>
	void clear_container_active(Container& items, T*& active) {
		if (!active || items.empty()) return;
		for (auto it = items.begin(); it != items.end(); ++it) {
			if (it->get() == active) {
				active = nullptr;
				items.erase(it);
				return;
			}
		}
	}

	template <typename Container, typename T>
	void clear_container_last(Container& items, T*& active) {
		if (items.empty()) return;
		if (active == items.back().get()) {
			active = nullptr;
		}
		items.pop_back();
	}

	template <typename Container, typename T>
	void cycle_container_active(Container& items, T*& active, int step = 1) {
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
		if (!found) {
			active = items[0].get();
			return;
		}
		auto size = static_cast<ssize_t>(items.size());
		ssize_t next_index = (static_cast<ssize_t>(current_index) + step) % size;
		if (next_index < 0) next_index += size;
		active = items[next_index].get();
	}
};

int run_application(int argc, char** argv, AppState& state, Config& config);
