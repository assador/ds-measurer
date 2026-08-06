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
	bool show_hypot{true};

	Measurement(Point p1, Point p2, const SelectionGuides& guides = SelectionGuides{});

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
		const Point& i_start,
		const Point& i_end,
		bool from_center,
		bool fixed_ratio,
		double ratio
	) {
		if (fixed_ratio) {
			double dx = i_end.x - i_start.x;
			double dy = i_end.y - i_start.y;
			double abs_dx = std::abs(dx);
			double abs_dy = std::abs(dy);

			if (abs_dx >= abs_dy) {
				end.x = i_start.x + static_cast<int>(std::copysign(abs_dy * ratio, dx));
				end.y = i_end.y;
			} else {
				end.x = i_end.x;
				end.y = i_start.y + static_cast<int>(std::copysign(abs_dx / ratio, dy));
			}
		} else {
			end = i_end;
		}
		if (from_center) {
			// with the end already calculated 
			start.x = i_start.x * 2 - end.x;
			start.y = i_start.y * 2 - end.y;
		} else {
			start = i_start;
		}
	}
};
