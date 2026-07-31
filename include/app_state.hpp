#pragma once

#include "cursor.hpp"
#include "measurement.hpp"
#include <vector>
#include <memory>

typedef struct _GtkWidget GtkWidget;

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

	std::vector<std::unique_ptr<Measurement>> frozen_measurements;
	std::unique_ptr<Measurement> draft_measurement;
	Measurement* active_measurement = nullptr;

	GtkWidget* drawing_area = nullptr;

	explicit AppState(Config& cfg) : config(cfg) {}

	void queue_draw() const;
};
