#pragma once

#include <cairo.h>
#include "core/cursor.hpp"
#include "core/grid.hpp"
#include "core/guide.hpp"
#include "core/measurement.hpp"
#include "core/types.hpp"

void set_label(cairo_t* cr, const TextStyle& style);

void draw_cursor(
	cairo_t* cr,
	const Cursor& c,
	int screen_w,
	int screen_h,
	const Color& color
);
void draw_grid(
	cairo_t* cr,
	const Grid& g,
	int x, int y, int w, int h,
	const Color& color
);

void draw_guide(
	cairo_t* cr,
	const Guide& g,
	int screen_w,
	int screen_h,
	const Color& color
);

void draw_measurement(
	cairo_t* cr,
	const Measurement& m,
	const ColorScheme& colors
);

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
	TextAlignH halign = TextAlignH::Left,
	TextAlignV valign = TextAlignV::Baseline
) {
	cairo_text_extents_t extents;
	cairo_text_extents(cr, text.c_str(), &extents);

	double rx = x;
	double ry = y;

	switch (halign) {
		case TextAlignH::Center:
			rx -= (extents.width / 2.0 + extents.x_bearing);
			break;
		case TextAlignH::Right:
			rx -= (extents.width + extents.x_bearing);
			break;
		case TextAlignH::Left:
			rx -= extents.x_bearing;
			break;
	}
	switch (valign) {
		case TextAlignV::Middle:
			ry -= (extents.height / 2.0 + extents.y_bearing);
			break;
		case TextAlignV::Bottom:
			ry -= (extents.height + extents.y_bearing);
			break;
		case TextAlignV::Top:
			ry -= extents.y_bearing;
			break;
		case TextAlignV::Baseline:
			break;
	}

	cairo_move_to(cr, rx, ry);
	cairo_show_text(cr, text.c_str());
}
