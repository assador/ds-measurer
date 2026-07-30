#pragma once

#include <cmath>
#include <string>

struct Color {
	double r{1.0};
	double g{1.0};
	double b{1.0};
	double a{1.0};
};

enum class GridType {
	CENTER,
	GOLDEN,
	THIRDS,
	FIFTHS,
	NONE
};

struct BoxConfig {
	bool show_center{false};
	bool show_golden{false};
	bool show_thirds{false};
	bool show_fifths{false};
	bool show_angle{true};
	bool show_bounding_box{true};
};

struct Point {
	int x{0};
	int y{0};

	Point(double dx, double dy) : x(static_cast<int>(dx)), y(static_cast<int>(dy)) {}
	Point(int ix, int iy) : x(ix), y(iy) {}
	Point() = default;

	double distance_to(const Point& other) const {
		return std::hypot(other.x - x, other.y - y);
	}
};

struct TextStyle {
	Color color = {1.0, 1.0, 1.0, 1.0};
	std::string font_family = "Sans";
	double font_size = 12.0;
};

enum class TextAlignH { LEFT, CENTER, RIGHT };
enum class TextAlignV { TOP, BASELINE, MIDDLE, BOTTOM };

struct TextOffset {
	double h{5.0}, v{5.0}, hs{3.0}, vs{3.0};
};
struct TextAlign {
	TextAlignH h{TextAlignH::LEFT}, hr{TextAlignH::RIGHT};
	TextAlignV v{TextAlignV::TOP}, vr{TextAlignV::BOTTOM};
};
