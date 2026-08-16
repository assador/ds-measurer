#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

enum class Orientation : std::uint8_t { Horizontal, Vertical };
enum class TextAlignH : std::uint8_t { Left, Center, Right };
enum class TextAlignV : std::uint8_t { Top, Baseline, Middle, Bottom };

enum class Mode : std::uint8_t { Measurements, Guides, Count };

struct Color {
	double r{0.0};
	double g{0.0};
	double b{0.0};
	double a{1.0};
};

struct ColorHSLA {
	double h{0.0};
	double s{0.0};
	double l{0.0};
	double a{1.0};
};

inline ColorHSLA rgb_to_hsl(double r, double g, double b, double a = 1.0) {
	double max_c = std::max({r, g, b});
	double min_c = std::min({r, g, b});
	double delta = max_c - min_c;

	double l = (max_c + min_c) / 2.0;
	double s = 0.0;
	double h = 0.0;

	if (delta > 0.00001) {
		s = (l <= 0.5) ? (delta / (max_c + min_c)) : (delta / (2.0 - max_c - min_c));

		if (max_c == r) {
			h = (g - b) / delta + (g < b ? 6.0 : 0.0);
		} else if (max_c == g) {
			h = (b - r) / delta + 2.0;
		} else {
			h = (r - g) / delta + 4.0;
		}
		h /= 6.0;
	}

	return ColorHSLA{.h = h * 360.0, .s = s, .l = l, .a = a};
}

inline ColorHSLA rgb_to_hsl(const Color& color) {
	return rgb_to_hsl(color.r, color.g, color.b, color.a);
}

struct ColorScheme {
	Color main{.r  = 0.0, .g = 0.5, .b = 1.0, .a = 0.5};
	Color basic{.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.25};
	Color guide{.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.15};
	Color highlight{.r = 0.0, .g = 0.5, .b = 1.0, .a = 1};
	Color text_main{.r = 0.0, .g = 0.5, .b = 1.0, .a = 0.6};
	Color text_basic{.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.75};
	Color text_faded{.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.4};
};

struct Point {
	int x{0};
	int y{0};

	Point(double dx, double dy)
		: x(static_cast<int>(std::round(dx)))
		, y(static_cast<int>(std::round(dy)))
	{}
	Point(int ix, int iy) : x(ix), y(iy) {}
	Point() = default;

	[[nodiscard]] double distance_to(const Point& other) const {
		return std::hypot(other.x - x, other.y - y);
	}
};

struct Rect {
	int x{0};
	int y{0};
	int w{0};
	int h{0};
};

struct SelectionGuideRule {
	std::vector<double> x;
	std::vector<double> y;
	bool show{false};
};

struct TextStyle {
	std::string font_family = "Sans";
	double font_size = 10.0;
};

struct TextOffset {
	double h{5.0}, v{5.0}, hs{3.0}, vs{3.0};
};
struct TextAlign {
	TextAlignH h{TextAlignH::Left}, hr{TextAlignH::Right};
	TextAlignV v{TextAlignV::Top}, vr{TextAlignV::Bottom};
};
