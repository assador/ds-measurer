#pragma once

#include "types.hpp"

typedef struct _cairo cairo_t;

void set_cairo_color(cairo_t* cr, const Color& color);

void draw_label(
	cairo_t* cr,
	const std::string& text,
	double x,
	double y,
	const TextStyle& style
);
