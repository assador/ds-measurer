#include "core/measurement.hpp"

Measurement::Measurement(Point p1, Point p2, const SelectionGuides& guides)
	: start(p1), end(p2) {
	grids.reserve(guides.size());
	for (const auto& [key, rule] : guides) {
		grids.emplace(key, Grid{rule});
	}
}
