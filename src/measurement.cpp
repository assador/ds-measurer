#include "measurement.hpp"
#include "ui.hpp"
#include <format>

Measurement::Measurement(Point p1, Point p2, const SelectionGuides& guides)
	: start(p1), end(p2) {
	grids.reserve(guides.size());
	for (const auto& [key, rule] : guides) {
		grids.emplace(key, Grid{rule});
	}
}

void Measurement::draw(cairo_t* cr, const ColorScheme& colors) const {
	cairo_set_line_width(cr, 1.0);

	int x_min = std::min(start.x, end.x);
	int x_max = std::max(start.x, end.x);
	int y_min = std::min(start.y, end.y);
	int y_max = std::max(start.y, end.y);

// SEC Hypot

	set_cairo_color(cr, colors.main);
	cairo_move_to(cr, coord_to_pixel(start.x), coord_to_pixel(start.y));
	cairo_line_to(cr, coord_to_pixel(end.x), coord_to_pixel(end.y));
	cairo_stroke(cr);

// SEC Bounding box

	if (box_config.show_bounding_box) {
		set_cairo_color(cr, colors.basic);
		cairo_rectangle(
			cr,
			coord_to_pixel(x_min),
			coord_to_pixel(y_min),
			x_max - x_min,
			y_max - y_min
		);
		cairo_stroke(cr);
	}

// SEC Labels

	double cx = (start.x + end.x) / 2.0;
	double cy = (start.y + end.y) / 2.0;

	TextAlign align;
	TextOffset offset;

	if (end.x >= start.x) {
		align.h = TextAlignH::LEFT; align.hr = TextAlignH::RIGHT;
		offset.h = 5.0;
	} else {
		align.h = TextAlignH::RIGHT; align.hr = TextAlignH::LEFT;
		offset.h = -5.0;
	}
	if (end.y >= start.y) {
		align.v = TextAlignV::BOTTOM; align.vr = TextAlignV::TOP;
		offset.v = 5.0;
	} else {
		align.v = TextAlignV::TOP; align.vr = TextAlignV::BOTTOM;
		offset.v = -5.0;
	}

	// hypot length
	std::string hypot_str = std::to_string(static_cast<int>(length()));
	set_cairo_color(cr, colors.text_main);
	draw_label(cr, hypot_str, cx + offset.h, cy - offset.v, align.h, align.v);

	// angle
	std::string angle_str = std::format("{:.6g} °", angle_deg());
	set_cairo_color(cr, colors.text_basic);
	draw_label(cr, angle_str, start.x + offset.h, start.y - offset.v, align.h, align.v);

	// width / height
	set_cairo_color(cr, colors.text_basic);
	draw_label(cr, std::to_string(width()), cx, end.y + offset.v, TextAlignH::CENTER, align.vr);
	draw_label(cr, std::to_string(height()), end.x + offset.h, cy, align.h, TextAlignV::MIDDLE);

	// coords of start / end points
	std::string coords_start_str = std::to_string(start.x) + ", " + std::to_string(start.y);
	std::string coords_end_str = std::to_string(end.x) + ", " + std::to_string(end.y);

	set_cairo_color(cr, colors.text_faded);
	draw_label(cr, coords_start_str, start.x - offset.h, start.y - offset.v, align.hr, align.v);
	set_cairo_color(cr, colors.text_main);
	draw_label(cr, coords_end_str, end.x + offset.h, end.y + offset.v, align.h, align.vr);

	// aspect ratio
	auto [rw, rh] = aspect_ratio();
	std::string ar_f_str = std::to_string(rw) + " : " + std::to_string(rh);
	std::string ar_n_str = std::format("{:.6g}", static_cast<double>(rw) / rh);

	set_cairo_color(cr, colors.text_faded);
	draw_label(cr, ar_f_str, end.x + offset.h, end.y + offset.v * 4, align.h, align.vr);
	draw_label(cr, ar_n_str, end.x + offset.h, end.y + offset.v * 7, align.h, align.vr);

// SEC Grids

	for (const auto& [_, grid] : grids) {
		grid.draw(cr, x_min, y_min, x_max - x_min, y_max - y_min, colors.basic);
	}
}
