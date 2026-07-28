#include "measurement.hpp"
#include "ui.hpp"
#include <cairo.h>

void Measurement::draw(cairo_t* cr) const {
	set_cairo_color(cr, color);

	cairo_set_line_width(cr, 1.5);
	cairo_move_to(cr, start.x, start.y);
	cairo_line_to(cr, end.x, end.y);
	cairo_stroke(cr);

	if (grid.show_bounding_box) {
		double dashes[] = {4.0, 4.0};
		cairo_set_dash(cr, dashes, 2, 0);
		cairo_set_line_width(cr, 1.0);
		cairo_rectangle(cr, start.x, start.y, end.x - start.x, end.y - start.y);
		cairo_stroke(cr);
		cairo_set_dash(cr, nullptr, 0, 0);
	}
}
