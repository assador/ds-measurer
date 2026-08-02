#pragma once

#include "core/types.hpp"

class Cursor {
public:
	Point pos;
	bool is_visible{true};

	void update(int x, int y) {
		pos.x = x;
		pos.y = y;
	}
};
