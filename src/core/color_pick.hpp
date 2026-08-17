#pragma once

#include "core/types.hpp"

class ColorPick {
public:
	Point pos;
	Color color;
	std::string fmt_str;

	explicit ColorPick(Point pos, Color color, std::string fmt_str = "");
};
