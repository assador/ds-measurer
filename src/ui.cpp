#include "ui.hpp"
#include <cairo.h>

void set_cairo_color(cairo_t* cr, const Color& color) {
	cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
}

void draw_label(
	cairo_t* cr,
	const std::string& text,
	double x,
	double y,
	const TextStyle& style
) {
	cairo_select_font_face(
		cr,
		style.font_family.c_str(),
		CAIRO_FONT_SLANT_NORMAL,
		CAIRO_FONT_WEIGHT_NORMAL
	);
	cairo_set_font_size(cr, style.font_size);

	cairo_text_extents_t extents;
	cairo_text_extents(cr, text.c_str(), &extents);

	cairo_set_source_rgba(cr, style.color.r, style.color.g, style.color.b, style.color.a);
	cairo_move_to(cr, x, y);
	cairo_show_text(cr, text.c_str());
}
