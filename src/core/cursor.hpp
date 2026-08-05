#pragma once

#include "core/types.hpp"

class Cursor {
public:
	Point pos;
	bool show{true};

	void update(int x, int y) {
		pos.x = x;
		pos.y = y;
	}
};
