#include "cursor.hpp"
#include "ui.hpp"
#include <cairo.h>

void Cursor::draw(cairo_t* cr, double screen_w, double screen_h) const {
	if (!is_visible) return;

	set_cairo_color(cr, color);

	cairo_set_line_width(cr, 1.0);
	cairo_move_to(cr, 0, pos.y);
	cairo_line_to(cr, screen_w, pos.y);
	cairo_move_to(cr, pos.x, 0);
	cairo_line_to(cr, pos.x, screen_h);
	cairo_stroke(cr);
}
