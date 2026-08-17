#include "core/color_pick.hpp"
#include "core/color.hpp"

ColorPick::ColorPick(Point pos, Color color, std::string fmt_str)
	: pos(pos)
	, color(color)
	, fmt_str(fmt_str.empty() ? utils::color_to_fmt_str(color) : std::move(fmt_str))
{}
