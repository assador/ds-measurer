#pragma once

#include "types.hpp"
#include <cairo.h>

void set_label(cairo_t* cr, const TextStyle& style);

inline double coord_to_pixel(int coord) {
	return static_cast<double>(coord) + 0.5;
}

inline void set_cairo_color(cairo_t* cr, const Color& color) {
	cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
}

inline void draw_label(
	cairo_t* cr,
	const std::string& text,
	double x,
	double y,
	TextAlignH halign = TextAlignH::LEFT,
	TextAlignV valign = TextAlignV::BASELINE
) {
	cairo_text_extents_t extents;
	cairo_text_extents(cr, text.c_str(), &extents);

	double rx = x;
	double ry = y;

	switch (halign) {
		case TextAlignH::CENTER:
			rx -= (extents.width / 2.0 + extents.x_bearing);
			break;
		case TextAlignH::RIGHT:
			rx -= (extents.width + extents.x_bearing);
			break;
		case TextAlignH::LEFT:
			rx -= extents.x_bearing;
			break;
	}
	switch (valign) {
		case TextAlignV::MIDDLE:
			ry -= (extents.height / 2.0 + extents.y_bearing);
			break;
		case TextAlignV::BOTTOM:
			ry -= (extents.height + extents.y_bearing);
			break;
		case TextAlignV::TOP:
			ry -= extents.y_bearing;
			break;
		case TextAlignV::BASELINE:
			break;
	}

	cairo_move_to(cr, rx, ry);
	cairo_show_text(cr, text.c_str());
}
