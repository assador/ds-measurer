#include "ui.hpp"

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
