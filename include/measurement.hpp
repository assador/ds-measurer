#pragma once

#include "types.hpp"
#include "config.hpp"
#include <cmath>
#include <numeric>
#include <utility>

typedef struct _cairo cairo_t;

class Measurement {
  public:
	Point start;
	Point end;
	GridConfig grid;
	bool is_frozen{false};

	Measurement(Point p1, Point p2) : start(p1), end(p2) {}

	int width() const { return std::abs(end.x - start.x) + 1; }
	int height() const { return std::abs(end.y - start.y) + 1; }
	double length() const { return start.distance_to(end); }

	double angle() const {
		return std::atan2(height(), width());
	}
	double angle_deg() const {
		return angle() * 180.0 / M_PI;
	}
	std::pair<int, int> aspect_ratio() const {
		int w = width();
		int h = height();
		if (w == 0 || h == 0) return {w, h};
		int g = std::gcd(w, h);
		return {w / g, h / g};
	}
	void draw(cairo_t* cr, const ColorScheme& colors) const;
};
