#pragma once

#include "types.hpp"

typedef struct _cairo cairo_t;

class Cursor {
public:
	Point pos;
	bool is_visible{true};

	void draw(cairo_t* cr, int screen_w, int screen_h, const Color& color) const;

	void update(int x, int y) {
		pos.x = x;
		pos.y = y;
	}
};
