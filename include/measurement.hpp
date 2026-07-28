#pragma once

#include "types.hpp"

typedef struct _cairo cairo_t;

class Measurement {
public:
	Point start;
	Point end;
	Color color{0.0, 0.0, 0.0, 1.0};
	GridConfig grid;
	bool is_frozen{false};

	Measurement(Point p1, Point p2) : start(p1), end(p2) {}

	void draw(cairo_t* cr) const;

	double width() const { return std::abs(end.x - start.x); }
	double height() const { return std::abs(end.y - start.y); }
	double length() const { return start.distance_to(end); }
};
