#pragma once

#include "cursor.hpp"
#include "measurement.hpp"
#include <vector>
#include <memory>

typedef struct _GtkWidget GtkWidget;

struct AppState {
	Config& config;
	Cursor cursor;
	std::vector<Measurement> frozen_measurements;
	std::unique_ptr<Measurement> active_measurement;

	GtkWidget* drawing_area = nullptr;

	explicit AppState(Config& cfg) : config(cfg) {}

	void queue_draw() const;
};
