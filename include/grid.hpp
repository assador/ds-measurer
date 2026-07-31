#pragma once

#include "config.hpp"

typedef struct _cairo cairo_t;

class Grid {
public:
	SelectionGuideRule rule;
	void draw(
		cairo_t* cr,
		int x, int y, int w, int h,
		const Color& color
	) const;
};
