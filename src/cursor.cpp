#include "cursor.hpp"
#include "ui.hpp"

void Cursor::draw(cairo_t* cr, int screen_w, int screen_h) const {
	if (!is_visible) return;

	cairo_set_line_width(cr, 1.0);
	set_cairo_color(cr, color);

	double px = coord_to_pixel(pos.x);
	double py = coord_to_pixel(pos.y);

	cairo_move_to(cr, 0, py);
	cairo_line_to(cr, screen_w, py);

	cairo_move_to(cr, px, 0);
	cairo_line_to(cr, px, screen_h);

	cairo_stroke(cr);
}
