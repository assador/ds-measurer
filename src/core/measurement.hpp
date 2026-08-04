#pragma once

#include <cmath>
#include <cstdint>
#include <numeric>
#include <utility>
#include "config/config.hpp"
#include "core/grid.hpp"
#include "core/types.hpp"

using Grids = std::unordered_map<uint32_t, Grid>;

class Measurement {
public:
	Point start;
	Point end;
	Grids grids;
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

	void move_from(const Point& p1, const Point& p2, int dx, int dy) {
		start = Point{p1.x + dx, p1.y + dy};
		end = Point{p2.x + dx, p2.y + dy};
	}

	void toggle_grid(uint32_t kc) {
		auto it = grids.find(kc);
		if (it == grids.end()) return;
		it->second.rule.show = !it->second.rule.show;
	}

	void apply_modifiers(
		const Point& source,
		const Point& target,
		bool from_center,
		bool fixed_ratio,
		double ratio
	) {
		if (fixed_ratio) {
			double dx = target.x - source.x;
			double dy = target.y - source.y;
			double abs_dx = std::abs(dx);
			double abs_dy = std::abs(dy);

			if (abs_dx >= abs_dy) {
				end.x = source.x + static_cast<int>(std::copysign(abs_dy * ratio, dx));
				end.y = target.y;
			} else {
				end.x = target.x;
				end.y = source.y + static_cast<int>(std::copysign(abs_dx / ratio, dy));
			}
		} else {
			end = target;
		}
		if (from_center) {
			start.x = source.x * 2 - end.x;
			start.y = source.y * 2 - end.y;
		} else {
			start = source;
		}
	}
};
