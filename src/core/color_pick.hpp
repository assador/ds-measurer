#pragma once

#include <optional>
#include "core/types.hpp"

struct ColorPickOptions {
	LabelState labels_state = { .show = true, .show_back = true };
};

class ColorPick {
public:
	Point pos;
	std::optional<Color> color;
	ColorPickOptions opts;
	std::string fmt_str;

	explicit ColorPick(
		Point pos,
		std::optional<Color> color = std::nullopt,
		ColorPickOptions opts = {},
		std::string fmt_str = "mmm…"
	);

	void move_by(int dx, int dy);
	void move_by(const Point& delta);
	void move_from(const Point& p0, int dx, int dy);
	void set_color(const std::optional<Color>& c);
	void hide_labels();
	void show_labels();
	void toggle_labels();
	void hide_labels_back();
	void show_labels_back();
	void toggle_labels_back();
};
