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
	
	std::vector<std::unique_ptr<Guide>> guides;
	std::vector<std::unique_ptr<Measurement>> measurements;
	std::unique_ptr<Measurement> draft_measurement;
	Guide* active_guide = nullptr;
	Measurement* active_measurement = nullptr;

	ColorScheme* color_scheme = nullptr;

	explicit AppState(Config& cfg) : config(cfg) {}

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
};

int run_application(int argc, char** argv, AppState& state, Config& config);
