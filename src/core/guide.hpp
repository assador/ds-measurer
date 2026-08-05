#pragma once

#include "core/types.hpp"

class Guide {
public:
	int position{0};
	Orientation orientation{Orientation::Horizontal};

	Guide(int pos, Orientation orient) : position(pos), orientation(orient) {}
};
