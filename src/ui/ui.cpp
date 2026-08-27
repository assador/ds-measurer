#include "ui/ui.hpp"
#include "core/types.hpp"
#include <format>
#include <string>

// SEC Color pick

void draw_color_pick(
	cairo_t* cr,
	const ColorPick& p,
	const ColorScheme& colors,
	const TextStyle& text_style
) {
	cairo_set_line_width(cr, 1.0);

	// SEC Labels

	if (p.opts.labels_state.show) {
		TextOffset offset;
		std::string coords_str = std::format("{}, {}", p.pos.x, p.pos.y);

		set_label(cr, text_style);
		LabelOpts opts = {
			.halign = TextAlignH::Left,
			.color = colors.text_faded,
			.color_back = colors.text_back,
			.show_back = p.opts.labels_state.show_back,
		};
		opts.valign = TextAlignV::Bottom;
		draw_label(cr, coords_str, p.pos.x + offset.h, p.pos.y - offset.v, opts);
		opts.valign = TextAlignV::Top;
		draw_multiline_label(cr, p.fmt_str, p.pos.x + offset.h, p.pos.y + offset.v, opts);
	}

	// SEC Cross

	const double cross_size = 20;
	const double px = coord_to_pixel(p.pos.x);
	const double py = coord_to_pixel(p.pos.y);

	set_cairo_color(cr, colors.guide);
	cairo_move_to(cr, p.pos.x - cross_size / 2, py);
	cairo_line_to(cr, p.pos.x + cross_size / 2, py);
	cairo_stroke(cr);
	cairo_move_to(cr, px, p.pos.y - cross_size / 2);
	cairo_line_to(cr, px, p.pos.y + cross_size / 2);
	cairo_stroke(cr);
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

	const double px = coord_to_pixel(c.pos.x);
	const double py = coord_to_pixel(c.pos.y);

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
		double px = coord_to_pixel(std::round(x + w * rx));
		cairo_move_to(cr, px, coord_to_pixel(y));
		cairo_line_to(cr, px, coord_to_pixel(y + h));
	}
	for (double ry : g.rule.y) {
		double py = coord_to_pixel(std::round(y + h * ry));
		cairo_move_to(cr, coord_to_pixel(x), py);
		cairo_line_to(cr, coord_to_pixel(x + w), py);
	}
	cairo_stroke(cr);
}

// SEC Guide

void draw_guide(
	cairo_t* cr,
	const Guide& g,
	int screen_w,
	int screen_h,
	const ColorScheme& colors
) {
	cairo_set_line_width(cr, 1.0);
	set_cairo_color(cr, colors.guide);

	const double px = coord_to_pixel(g.coord);

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
	const ColorScheme& colors,
	const TextStyle& text_style
) {
	cairo_set_line_width(cr, 1.0);

	const int x_min = std::min(m.start.x, m.end.x);
	const int x_max = std::max(m.start.x, m.end.x);
	const int y_min = std::min(m.start.y, m.end.y);
	const int y_max = std::max(m.start.y, m.end.y);

	int dx = m.end.x - m.start.x;
	int dy = m.end.y - m.start.y;

	double phi = std::atan2(dy, dx);
	if (dx < 0 && dy < 0) phi += 2 * M_PI;
	double base = (dx >= 0) ? 0.0 : M_PI;

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

	// SEC Hypot and arc

	if (m.opts.show_diagonal) {
		// diagonal
		set_cairo_color(cr, colors.main);
		cairo_move_to(cr, coord_to_pixel(m.start.x), coord_to_pixel(m.start.y));
		cairo_line_to(cr, coord_to_pixel(m.end.x), coord_to_pixel(m.end.y));
		cairo_stroke(cr);
		// arc
		if (m.width(true) > 20) {
			set_cairo_color(cr, colors.basic);
			cairo_arc(cr, m.start.x, m.start.y, 20, std::min(base, phi), std::max(base, phi));
			cairo_stroke(cr);
		}
	}

	// SEC Labels

	if (!m.opts.labels_state.show) return;

	const double cx = (m.start.x + m.end.x) / 2.0;
	const double cy = (m.start.y + m.end.y) / 2.0;

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
	LabelOpts opts = {
		.halign = align.h,
		.valign = align.v,
		.color = colors.text_basic,
		.color_back = colors.text_back,
		.show_back = m.opts.labels_state.show_back,
	};

	set_label(cr, text_style);

	if (m.opts.show_diagonal) {
		// diagonal length
		std::string diagonal_str = std::to_string(static_cast<int>(m.length(true)));
		opts.color = colors.text_main;
		draw_label(cr, diagonal_str, cx + offset.h, cy - offset.v, opts);
		// angle
		std::string angle_str = std::format("{:.6g} °", m.angle_deg(true));
		opts.color = colors.text_basic;
		draw_label(cr, angle_str, m.start.x + offset.h, m.start.y - offset.v, opts);
	}

	// width / height
	opts.color = colors.text_basic;
	opts.halign = TextAlignH::Center;
	opts.valign = align.vr;
	draw_label(cr, std::to_string(m.width(true)), cx, m.end.y + offset.v, opts);
	opts.halign = align.h;
	opts.valign = TextAlignV::Middle;
	draw_label(cr, std::to_string(m.height(true)), m.end.x + offset.h, cy, opts);

	// coords of start / end points
	std::string coords_start_str = std::format(
		"{}, {}",
		m.start.x + (dx < 0 ? 1 : 0),
		m.start.y + (dy < 0 ? 1 : 0)
	);
	std::string coords_end_str = std::format(
		"{}, {}",
		m.end.x + (dx >= 0 ? 1 : 0),
		m.end.y + (dy >= 0 ? 1 : 0)
	);
	// aspect ratio
	auto [rw, rh] = m.aspect_ratio(true);
	std::string ar_f_str = std::format("{} : {}", rw, rh);
	std::string ar_n_str = std::format("{:.6g}", static_cast<double>(rw) / rh);

	opts.color = colors.text_faded;
	opts.halign = align.hr;
	opts.valign = align.v;
	draw_label(cr, coords_start_str, m.start.x - offset.h, m.start.y - offset.v, opts);

	const std::string fmt_str = std::format("{}\n{}\n{}", coords_end_str, ar_f_str, ar_n_str);
	opts.halign = align.h;
	opts.valign = align.vr;
	draw_multiline_label(cr, fmt_str, m.end.x + offset.h, m.end.y + offset.v, opts);

	// SEC Grids

	for (const auto& [_, grid] : m.grids) {
		draw_grid(cr, grid, x_min, y_min, x_max - x_min, y_max - y_min, colors.guide);
	}
}
