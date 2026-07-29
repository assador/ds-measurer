#include "measurement.hpp"
#include "ui.hpp"

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

	if (grid.show_bounding_box) {
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
	double offset_w = (end.y >= start.y) ? 5.0 : -5.0;
	double offset_h = (end.x >= start.x) ? 5.0 : -5.0;

	TextHalign halign = (end.x >= start.x) ? TextHalign::RIGHT : TextHalign::LEFT;
	TextValign valign = (end.y >= start.y) ? TextValign::BOTTOM : TextValign::TOP;
	TextHalign halign_r = (end.x >= start.x) ? TextHalign::LEFT : TextHalign::RIGHT;
	TextValign valign_r = (end.y >= start.y) ? TextValign::TOP : TextValign::BOTTOM;

	set_cairo_color(cr, colors.text_basic);
	draw_label(cr, std::to_string(width()), cx, end.y + offset_w, TextHalign::CENTER, valign_r);
	draw_label(cr, std::to_string(height()), end.x + offset_h, cy, halign_r, TextValign::MIDDLE);

	std::string coords_start_str = std::to_string(start.x) + ", " + std::to_string(start.y);
	std::string coords_end_str = std::to_string(end.x) + ", " + std::to_string(end.y);

	set_cairo_color(cr, colors.text_faded);
	draw_label(cr, coords_start_str, start.x - offset_w, start.y - offset_h, halign, valign);
	draw_label(cr, coords_end_str, end.x + offset_w, end.y + offset_h, halign_r, valign_r);

	auto [rw, rh] = aspect_ratio();
	std::string info_str =
		std::to_string(static_cast<int>(length())) + "px (" +
		std::to_string(rw) + ":" + std::to_string(rh) + ")"
	;
	draw_label(cr, info_str, cx, cy);
}
