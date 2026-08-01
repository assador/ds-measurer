#pragma once

#include "types.hpp"
#include "config.hpp"
#include "grid.hpp"
#include <cmath>
#include <numeric>
#include <utility>

typedef struct _cairo cairo_t;

class Measurement {
public:
	Point start;
	Point end;
	BoxConfig box_config;
	std::vector<Grid> grids;
	bool is_changing{false};
	bool is_moving{false};
	bool is_hypot_visible{true};

	Measurement(Point p1, Point p2, const SelectionGuides& guides = SelectionGuides{});

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
	void move_by(int dx, int dy) {
		start.x += dx;
		start.y += dy;
		end.x += dx;
		end.y += dy;
	}
	void move_by(const Point& delta) { move_by(delta.x, delta.y); }

	void draw(cairo_t* cr, const ColorScheme& colors) const;
};
