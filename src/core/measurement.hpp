#pragma once

#include <cmath>
#include <cstdint>
#include <numeric>
#include <utility>
#include "config/config.hpp"
#include "core/grid.hpp"
#include "core/types.hpp"

using Grids = std::unordered_map<uint32_t, Grid>;

struct MeasurementOptions {
	bool show_diagonal{true};
};

class Measurement {
public:
	Point start;
	Point end;
	Grids grids;
	bool show_diagonal{true};

	Measurement(
		Point p1,
		Point p2,
		MeasurementOptions opts = {},
		const SelectionGuides& guides = SelectionGuides{}
	);

	int width(bool greedy = false) const {
		return std::abs(end.x - start.x) + (greedy ? 1 : 0);
	}

	int height(bool greedy = false) const {
		return std::abs(end.y - start.y) + (greedy ? 1 : 0);
	}

	double length(bool greedy = false) const {
		return std::hypot(width(greedy), height(greedy));
	}

	double angle(bool greedy = false) const {
		return std::atan2(height(greedy), width(greedy));
	}

	double angle_deg(bool greedy = false) const {
		return angle(greedy) * 180.0 / M_PI;
	}

	std::pair<int, int> aspect_ratio(bool greedy = false) const {
		int w = width(greedy);
		int h = height(greedy);
		if (w == 0 || h == 0) return {w, h};
		int g = std::gcd(w, h);
		return {w / g, h / g};
	}

	void toggle_grid(uint32_t kc);
	void move_by(int dx, int dy);
	void move_by(const Point& delta);
	void move_from(const Point& p1, const Point& p2, int dx, int dy);
	void apply_modifiers(
		const Point& i_start,
		const Point& i_end,
		bool from_center,
		bool fixed_ratio,
		double ratio
	);
};
