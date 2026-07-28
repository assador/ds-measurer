#pragma once

#include <cmath>
#include <string>

struct Color {
	double r{1.0};
	double g{1.0};
	double b{1.0};
	double a{1.0};
};

struct GridConfig {
	bool show_thirds{false};
	bool show_golden{false};
	bool show_fifths{false};
	bool show_angle{true};
	bool show_bounding_box{true};
};

struct Point {
	double x{0.0};
	double y{0.0};

	double distance_to(const Point& other) const {
		return std::hypot(other.x - x, other.y - y);
	}
};

struct TextStyle {
	Color color = {1.0, 1.0, 1.0, 1.0};
	std::string font_family = "Sans";
	double font_size = 12.0;
};
