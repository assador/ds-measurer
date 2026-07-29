#pragma once

#include "cursor.hpp"
#include "measurement.hpp"
#include <vector>
#include <memory>

struct AppState {
	Config& config;
	Cursor cursor;
	std::vector<Measurement> frozen_measurements;
	std::unique_ptr<Measurement> active_measurement;
	explicit AppState(Config& cfg) : config(cfg) {}
};
