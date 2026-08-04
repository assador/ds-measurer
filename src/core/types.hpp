#pragma once

#include <cmath>
#include <cstdint>
#include <string>

struct Color {
	double r{0.0};
	double g{0.0};
	double b{0.0};
	double a{1.0};
};

enum class GridType : std::uint8_t {
	CENTER,
	GOLDEN,
	THIRDS,
	FIFTHS,
	NONE
};

struct Point {
	int x{0};
	int y{0};

	Point(double dx, double dy) : x(static_cast<int>(dx)), y(static_cast<int>(dy)) {}
	Point(int ix, int iy) : x(ix), y(iy) {}
	Point() = default;

	[[nodiscard]] double distance_to(const Point& other) const {
		return std::hypot(other.x - x, other.y - y);
	}
};

struct TextStyle {
	Color color = {.r=0.0, .g=0.0, .b=0.0, .a=1.0};
	std::string font_family = "Sans";
	double font_size = 12.0;
};

enum class TextAlignH : std::uint8_t { LEFT, CENTER, RIGHT };
enum class TextAlignV : std::uint8_t { TOP, BASELINE, MIDDLE, BOTTOM };

struct TextOffset {
	double h{5.0}, v{5.0}, hs{3.0}, vs{3.0};
};
struct TextAlign {
	TextAlignH h{TextAlignH::LEFT}, hr{TextAlignH::RIGHT};
	TextAlignV v{TextAlignV::TOP}, vr{TextAlignV::BOTTOM};
};
