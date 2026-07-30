#include "grid.hpp"
#include "ui.hpp"
#include <cmath>

void Grid::draw(
	cairo_t* cr,
	int x, int y, int w, int h,
	const ColorScheme& colors
) const {
	if (!rule.show || w <= 0 || h <= 0) return;

	cairo_set_line_width(cr, 1.0);
	set_cairo_color(cr, colors.basic);

	for (double rx : rule.x) {
		int px = std::lround(x + w * rx);
		cairo_move_to(cr, coord_to_pixel(px), coord_to_pixel(y));
		cairo_line_to(cr, coord_to_pixel(px), coord_to_pixel(y + h));
	}
	for (double ry : rule.y) {
		int py = std::lround(y + h * ry);
		cairo_move_to(cr, coord_to_pixel(x), coord_to_pixel(py));
		cairo_line_to(cr, coord_to_pixel(x + w), coord_to_pixel(py));
	}
	cairo_stroke(cr);
}
