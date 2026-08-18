#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include "config/config.hpp"
#include "core/cursor.hpp"
#include "core/guide.hpp"
#include "core/color_pick.hpp"
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

	bool show_color_picks{true};
	bool show_guides{true};
	bool snap_to_guides{true};
	
	std::vector<std::unique_ptr<ColorPick>> color_picks;
	std::vector<std::unique_ptr<Guide>> guides;
	std::vector<std::unique_ptr<Measurement>> measurements;
	std::unique_ptr<Measurement> draft_measurement;
	ColorPick* active_color_pick = nullptr;
	Guide* active_guide = nullptr;
	Measurement* active_measurement = nullptr;

	ColorScheme* color_scheme = nullptr;

	explicit AppState(Config& cfg) : config(cfg) {}

	std::function<void()> request_redraw;
	void redraw() const {
		if (request_redraw) request_redraw();
	}
	std::function<void(const std::string&)> request_text_to_clipboard;
	void text_to_clipboard(const std::string& text) const {
		if (request_text_to_clipboard) request_text_to_clipboard(text);
	}
	std::function<void(int x, int y, int w, int h)> request_screenshot;
	void screenshot(Measurement* m) const {
		if (!m) return;
		auto r = m->normalized_rect(true);
		if (request_screenshot) request_screenshot(r.x, r.y, r.w, r.h);
	}
	std::function<std::optional<Color>(int x, int y)> request_get_pixel_color;
	[[nodiscard]] std::optional<Color> get_pixel_color(int x, int y) const {
		if (request_get_pixel_color) return request_get_pixel_color(x, y);
		return std::nullopt;
	}
	std::function<void(Color& color)> request_color_to_clipboard;
	void color_to_clipboard(Color& color) const {
		if (request_color_to_clipboard) request_color_to_clipboard(color);
	}

	void request_quit();

	void apply_from_config();
	void set_color_scheme(const std::string& name);

	void add_guide(Orientation orient);
	void clear_active();
	void clear_all();
	void clear_last();
	void cycle_active(int step = 1);
	void relax();
	void copy_values(Measurement* m);
	void copy_active_color();
	void freeze_draft();
	void pick_color(Cursor& c);

	[[nodiscard]] Point get_snapped_pos(Point initial) const;
	void redraw_measurement_with_modifiers(Measurement& m);
	void set_fixed_ratio(bool is_pressed);
	void set_from_center(bool is_pressed);
	void set_ratio(double r);
	void snap_active_translation();
	void toggle_grids_of_active(uint32_t kc);
	void toggle_diagonal_of_active();

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
