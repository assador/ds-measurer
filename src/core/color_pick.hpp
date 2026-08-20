#pragma once

#include <optional>
#include "core/types.hpp"

class ColorPick {
public:
	Point pos;
	std::optional<Color> color;
	std::string fmt_str;

	explicit ColorPick(
		Point pos,
		std::optional<Color> color = std::nullopt,
		std::string fmt_str = "mmm…"
	);

	void move_by(int dx, int dy);
	void move_by(const Point& delta);
	void move_from(const Point& p0, int dx, int dy);
	void set_color(const std::optional<Color>& c);
};
