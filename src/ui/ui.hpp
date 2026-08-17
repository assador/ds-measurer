#pragma once

#include <cairo.h>
#include <sstream>
#include "core/color_pick.hpp"
#include "core/cursor.hpp"
#include "core/grid.hpp"
#include "core/guide.hpp"
#include "core/measurement.hpp"
#include "core/types.hpp"

void draw_color_pick(
	cairo_t* cr,
	const ColorPick& p,
	const ColorScheme& colors,
	const TextStyle& text_style
);
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
	const ColorScheme& colors,
	const TextStyle& text_style
);

inline double coord_to_pixel(double coord) {
	return coord + 0.5;
}

inline void set_cairo_color(cairo_t* cr, const Color& color) {
	cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
}

inline void set_label(
	cairo_t* cr,
	const TextStyle& style
) {
	cairo_font_options_t* font_opts = cairo_font_options_create();
	cairo_get_font_options(cr, font_opts);
	cairo_font_options_set_antialias(font_opts, CAIRO_ANTIALIAS_GRAY);
	cairo_font_options_set_subpixel_order(font_opts, CAIRO_SUBPIXEL_ORDER_DEFAULT);
	cairo_set_font_options(cr, font_opts);
	cairo_font_options_destroy(font_opts);

	cairo_select_font_face(
		cr,
		style.font_family.c_str(),
		CAIRO_FONT_SLANT_NORMAL,
		CAIRO_FONT_WEIGHT_NORMAL
	);
	cairo_set_font_size(cr, style.font_size);
}

inline void draw_label(
	cairo_t* cr,
	const std::string& text,
	double x,
	double y,
	TextAlignH halign = TextAlignH::Left,
	TextAlignV valign = TextAlignV::Baseline
) {
	if (text.empty()) return;

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

inline void draw_multiline_label(
	cairo_t* cr,
	const std::string& text,
	double x,
	double y,
	TextAlignH halign = TextAlignH::Left,
	TextAlignV valign = TextAlignV::Baseline
) {
	if (text.empty()) return;

	if (text.find('\n') == std::string::npos) {
		draw_label(cr, text, x, y, halign, valign);
		return;
	}

	std::vector<std::string> lines;
	std::stringstream ss(text);
	std::string line;
	while (std::getline(ss, line, '\n')) {
		lines.push_back(line);
	}

	cairo_font_extents_t fe;
	cairo_font_extents(cr, &fe);
	const double line_height = fe.height;
	const double total_height = line_height * static_cast<double>(lines.size());

	double start_y = y;
	switch (valign) {
		case TextAlignV::Middle:
			start_y -= (total_height / 2.0 - fe.ascent);
			break;
		case TextAlignV::Bottom:
			start_y -= (total_height - fe.ascent);
			break;
		case TextAlignV::Top:
			start_y += fe.ascent;
			break;
		case TextAlignV::Baseline:
			break;
	}
	for (size_t i = 0; i < lines.size(); ++i) {
		const double current_y = start_y + static_cast<double>(i) * line_height;
		draw_label(cr, lines[i], x, current_y, halign, TextAlignV::Baseline);
	}
}
