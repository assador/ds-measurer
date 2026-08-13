#pragma once

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

struct ColorScheme {
	Color main{.r=0.0, .g=0.5, .b=1.0, .a=0.5};
	Color basic{.r=0.0, .g=0.0, .b=0.0, .a=0.25};
	Color guide{.r=0.0, .g=0.0, .b=0.0, .a=0.15};
	Color highlight{.r=0.0, .g=0.5, .b=1.0, .a=1};
	Color text_main{.r=0.0, .g=0.5, .b=1.0, .a=0.6};
	Color text_basic{.r=0.0, .g=0.0, .b=0.0, .a=0.75};
	Color text_faded{.r=0.0, .g=0.0, .b=0.0, .a=0.4};
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
