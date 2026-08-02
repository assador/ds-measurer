#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>
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

	std::optional<double> ratio = std::nullopt;
	std::string active_ratio_key;

	std::vector<std::unique_ptr<Measurement>> frozen_measurements;
	std::unique_ptr<Measurement> draft_measurement;
	Measurement* active_measurement = nullptr;

	explicit AppState(Config& cfg) : config(cfg) {}

	void reset_ratio() {
		ratio = std::nullopt;
		active_ratio_key.clear();
	}
	void toggle_ratio(const std::string& key) {
		if (active_ratio_key == key && ratio.has_value()) {
			reset_ratio();
			return;
		}
		auto it = config.ratios.find(key);
		if (it == config.ratios.end()) return;
		ratio = it->second;
		active_ratio_key = key;
	}

	std::function<void()> request_draw;
	void queue_draw() const {
		if (request_draw) request_draw();
	}
};
