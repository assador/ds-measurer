#include "ui/ui.hpp"
#include <format>
#include <string>

void set_label(
	cairo_t* cr,
	const TextStyle& style
) {
	set_cairo_color(cr, style.color);

	cairo_select_font_face(
		cr,
		style.font_family.c_str(),
		CAIRO_FONT_SLANT_NORMAL,
		CAIRO_FONT_WEIGHT_NORMAL
	);
	cairo_set_font_size(cr, style.font_size);
}

// SEC Cursor

void draw_cursor(
	cairo_t* cr,
	const Cursor& c,
	int screen_w,
	int screen_h,
	const Color& color
) {
	if (!c.show) return;

	cairo_set_line_width(cr, 1.0);
	set_cairo_color(cr, color);

	double px = coord_to_pixel(c.pos.x);
	double py = coord_to_pixel(c.pos.y);

	cairo_move_to(cr, 0, py);
	cairo_line_to(cr, screen_w, py);

	cairo_move_to(cr, px, 0);
	cairo_line_to(cr, px, screen_h);

	cairo_stroke(cr);
}

// SEC Grid

void draw_grid(
	cairo_t* cr,
	const Grid& g,
	int x, int y, int w, int h,
	const Color& color
) {
	if (!g.rule.show || w <= 0 || h <= 0) return;

	cairo_set_line_width(cr, 1.0);
	set_cairo_color(cr, color);

	for (double rx : g.rule.x) {
		int px = static_cast<int>(std::round(x + w * rx));
		cairo_move_to(cr, coord_to_pixel(px), y);
		cairo_line_to(cr, coord_to_pixel(px), y + h);
	}
	for (double ry : g.rule.y) {
		int py = static_cast<int>(std::round(y + h * ry));
		cairo_move_to(cr, x, coord_to_pixel(py));
		cairo_line_to(cr, x + w, coord_to_pixel(py));
	}
	cairo_stroke(cr);
}

// SEC Guide

void draw_guide(
	cairo_t* cr,
	const Guide& g,
	int screen_w,
	int screen_h,
	const Color& color
) {
	cairo_set_line_width(cr, 1.0);
	set_cairo_color(cr, color);

	double px = coord_to_pixel(g.position);

	if (g.orientation == Orientation::Horizontal) {
		cairo_move_to(cr, 0, px);
		cairo_line_to(cr, screen_w, px);
	} else if (g.orientation == Orientation::Vertical) {
		cairo_move_to(cr, px, 0);
		cairo_line_to(cr, px, screen_h);
	}
	cairo_stroke(cr);
}

// SEC Measurement

void draw_measurement(
	cairo_t* cr,
	const Measurement& m,
	const ColorScheme& colors
) {
	cairo_set_line_width(cr, 1.0);

	int x_min = std::min(m.start.x, m.end.x);
	int x_max = std::max(m.start.x, m.end.x);
	int y_min = std::min(m.start.y, m.end.y);
	int y_max = std::max(m.start.y, m.end.y);

	int dx = m.end.x - m.start.x;
	int dy = m.end.y - m.start.y;

	double phi = std::atan2(dy, dx);
	if (dx < 0 && dy < 0) phi += 2 * M_PI;
	double base = (dx >= 0) ? 0.0 : M_PI;

	// SEC Hypot and arc

	if (m.show_hypot) {
		// hypot
		set_cairo_color(cr, colors.main);
		cairo_move_to(cr, coord_to_pixel(m.start.x), coord_to_pixel(m.start.y));
		cairo_line_to(cr, coord_to_pixel(m.end.x), coord_to_pixel(m.end.y));
		cairo_stroke(cr);
		// arc
		if (m.width() > 20) {
			set_cairo_color(cr, colors.basic);
			cairo_arc(cr, m.start.x, m.start.y, 20, std::min(base, phi), std::max(base, phi));
			cairo_stroke(cr);
		}
	}

	// SEC Bounding box

	set_cairo_color(cr, colors.basic);
	cairo_rectangle(
		cr,
		coord_to_pixel(x_min),
		coord_to_pixel(y_min),
		x_max - x_min,
		y_max - y_min
	);
	cairo_stroke(cr);

	// SEC Labels

	double cx = (m.start.x + m.end.x) / 2.0;
	double cy = (m.start.y + m.end.y) / 2.0;

	TextAlign align;
	TextOffset offset;

	if (m.end.x >= m.start.x) {
		align.h = TextAlignH::Left; align.hr = TextAlignH::Right;
		offset.h = 5.0;
	} else {
		align.h = TextAlignH::Right; align.hr = TextAlignH::Left;
		offset.h = -5.0;
	}
	if (m.end.y >= m.start.y) {
		align.v = TextAlignV::Bottom; align.vr = TextAlignV::Top;
		offset.v = 5.0;
	} else {
		align.v = TextAlignV::Top; align.vr = TextAlignV::Bottom;
		offset.v = -5.0;
	}

	if (m.show_hypot) {
		// hypot length
		std::string hypot_str = std::to_string(static_cast<int>(m.length()));
		set_cairo_color(cr, colors.text_main);
		draw_label(cr, hypot_str, cx + offset.h, cy - offset.v, align.h, align.v);
		// angle
		std::string angle_str = std::format("{:.6g} °", m.angle_deg());
		set_cairo_color(cr, colors.text_basic);
		draw_label(cr, angle_str, m.start.x + offset.h, m.start.y - offset.v, align.h, align.v);
	}

	// width / height
	set_cairo_color(cr, colors.text_basic);
	draw_label(cr, std::to_string(m.width()), cx, m.end.y + offset.v, TextAlignH::Center, align.vr);
	draw_label(cr, std::to_string(m.height()), m.end.x + offset.h, cy, align.h, TextAlignV::Middle);

	// coords of start / end points
	std::string coords_start_str = std::to_string(m.start.x) + ", " + std::to_string(m.start.y);
	std::string coords_end_str = std::to_string(m.end.x) + ", " + std::to_string(m.end.y);

	set_cairo_color(cr, colors.text_faded);
	draw_label(cr, coords_start_str, m.start.x - offset.h, m.start.y - offset.v, align.hr, align.v);
	set_cairo_color(cr, colors.text_main);
	draw_label(cr, coords_end_str, m.end.x + offset.h, m.end.y + offset.v, align.h, align.vr);

	// aspect ratio
	auto [rw, rh] = m.aspect_ratio();
	std::string ar_f_str = std::to_string(rw) + " : " + std::to_string(rh);
	std::string ar_n_str = std::format("{:.6g}", static_cast<double>(rw) / rh);

	set_cairo_color(cr, colors.text_faded);
	draw_label(cr, ar_f_str, m.end.x + offset.h, m.end.y + offset.v * 4, align.h, align.vr);
	draw_label(cr, ar_n_str, m.end.x + offset.h, m.end.y + offset.v * 7, align.h, align.vr);

	// SEC Grids

	for (const auto& [_, grid] : m.grids) {
		draw_grid(cr, grid, x_min, y_min, x_max - x_min, y_max - y_min, colors.guide);
	}
}
