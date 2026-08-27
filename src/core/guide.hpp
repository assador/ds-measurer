#pragma once

#include "core/types.hpp"

class Guide {
public:
	int coord = 0;
	Orientation orientation = Orientation::Horizontal;

	Guide(int c, Orientation o) : coord(c), orientation(o) {}
};
