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
	const ColorScheme& colors
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
	LabelOpts opts
) {
	if (text.empty()) return;

	cairo_text_extents_t te;
	cairo_text_extents(cr, text.c_str(), &te);
	cairo_font_extents_t fe;
	cairo_font_extents(cr, &fe);

	const double font_h = fe.ascent + fe.descent;

	const double rect_w = te.width + opts.padding.l + opts.padding.r;
	const double rect_h = font_h + opts.padding.t + opts.padding.b;

	double rect_x = x;
	double rect_y = y;

	switch (opts.halign) {
		case TextAlignH::Left:
			rect_x = x;
			break;
		case TextAlignH::Center:
			rect_x = x - rect_w / 2.0;
			break;
		case TextAlignH::Right:
			rect_x = x - rect_w;
			break;
	}
	switch (opts.valign) {
		case TextAlignV::Top:
			rect_y = y;
			break;
		case TextAlignV::Middle:
			rect_y = y - rect_h / 2.0;
			break;
		case TextAlignV::Bottom:
			rect_y = y - rect_h;
			break;
		case TextAlignV::Baseline:
			rect_y = y - fe.ascent - opts.padding.t;
			break;
	}

	if (opts.show_back) {
		set_cairo_color(cr, opts.color_back);
		cairo_rectangle(cr, rect_x, rect_y, rect_w, rect_h);
		cairo_fill(cr);
	}

	const double rx = rect_x + opts.padding.l - te.x_bearing;
	const double ry = rect_y + opts.padding.t + fe.ascent;

	set_cairo_color(cr, opts.color);
	cairo_move_to(cr, rx, ry);
	cairo_show_text(cr, text.c_str());
}

inline void draw_multiline_label(
	cairo_t* cr,
	const std::string& text,
	double x,
	double y,
	LabelOpts opts
) {
	if (text.empty()) return;

	if (text.find('\n') == std::string::npos) {
		draw_label(cr, text, x, y, opts);
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
	const double font_h = fe.ascent + fe.descent;
	const double line_h = fe.height;

	double text_w = 0.0;
	for (const auto& l : lines) {
		cairo_text_extents_t te;
		cairo_text_extents(cr, l.c_str(), &te);
		if (te.width > text_w) text_w = te.width;
	}

	const double text_h = line_h * static_cast<double>(lines.size());
	const double rect_w = text_w + opts.padding.l + opts.padding.r;
	const double rect_h = text_h + opts.padding.t + opts.padding.b;

	double rect_x = x;
	double rect_y = y;

	switch (opts.halign) {
		case TextAlignH::Left:
			rect_x = x;
			break;
		case TextAlignH::Center:
			rect_x = x - rect_w / 2.0;
			break;
		case TextAlignH::Right:
			rect_x = x - rect_w;
			break;
	}
	switch (opts.valign) {
		case TextAlignV::Top:
			rect_y = y;
			break;
		case TextAlignV::Middle:
			rect_y = y - rect_h / 2.0;
			break;
		case TextAlignV::Bottom:
			rect_y = y - rect_h;
			break;
		case TextAlignV::Baseline:
			rect_y = y - fe.ascent - opts.padding.t;
			break;
	}

	if (opts.show_back) {
		set_cairo_color(cr, opts.color_back);
		cairo_rectangle(cr, rect_x, rect_y, rect_w, rect_h);
		cairo_fill(cr);
	}

	auto line_opts = opts;
	line_opts.show_back = false;
	line_opts.valign = TextAlignV::Top;
	line_opts.padding = { .t = 0, .r = 0, .b = 0, .l = 0 };

	const double content_y = rect_y + opts.padding.t;

	for (size_t i = 0; i < lines.size(); ++i) {
		const double line_y = content_y + static_cast<double>(i) * line_h;

		double line_x = rect_x + opts.padding.l;
		if (opts.halign == TextAlignH::Center) {
			line_x = rect_x + rect_w / 2.0;
		} else if (opts.halign == TextAlignH::Right) {
			line_x = rect_x + rect_w - opts.padding.r;
		}

		draw_label(cr, lines[i], line_x, line_y, line_opts);
	}
}
