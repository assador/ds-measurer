#include "core/color_pick.hpp"
#include "core/color.hpp"

ColorPick::ColorPick(
	Point pos,
	std::optional<Color> color,
	std::string fmt_str
)
	: pos(pos)
	, color(color)
	, fmt_str(color ? utils::color_to_fmt_str(*color) : std::move(fmt_str))
{}

void ColorPick::move_by(int dx, int dy) {
	pos.x += dx;
	pos.y += dy;
}
void ColorPick::move_by(const Point& delta) { move_by(delta.x, delta.y); }

void ColorPick::move_from(const Point& p0, int dx, int dy) {
	pos = Point{p0.x + dx, p0.y + dy};
}

void ColorPick::set_color(const std::optional<Color>& c) {
	color = c;
	fmt_str = color ? utils::color_to_fmt_str(*color) : "mmm…";
}
