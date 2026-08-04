#pragma once

#include <functional>
#include <memory>
#include <vector>
#include "config/config.hpp"
#include "core/cursor.hpp"
#include "core/measurement.hpp"

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

	double ratio{1.1};
	bool is_pressed_fixed_ratio{false};
	bool is_pressed_from_center{false};

	std::vector<std::unique_ptr<Measurement>> frozen_measurements;
	std::unique_ptr<Measurement> draft_measurement;
	Measurement* active_measurement = nullptr;

	explicit AppState(Config& cfg) : config(cfg) {}

	std::function<void()> request_draw;
	void queue_draw() const {
		if (request_draw) request_draw();
	}
	void request_quit();
};

int run_application(int argc, char** argv, AppState& state, Config& config);
