#pragma once

#include <cairo.h>
#include <cmath>
#include <memory>
#include <vector>

struct Point {
	double x{0.0};
	double y{0.0};

	double distance_to(const Point& other) const {
		double dx = other.x - x;
		double dy = other.y - y;
		return std::sqrt(dx * dx + dy * dy);
	}
};

struct Color {
	double r{1.0};
	double g{1.0};
	double b{1.0};
	double a{1.0};

	void apply_to_cairo(cairo_t* cr) const {
		cairo_set_source_rgba(cr, r, g, b, a);
	}
};

struct GridConfig {
	bool show_thirds{false};
	bool show_golden{false};
	bool show_fifths{false};
	bool show_angle{true};
	bool show_bounding_box{true};
};

class Measurement {
public:
	Point start;
	Point end;
	Color color{0.0, 0.0, 0.0, 1.0};
	GridConfig grid;
	bool is_frozen{false};

	Measurement(Point p1, Point p2) : start(p1), end(p2) {}

	double width() const { return std::abs(end.x - start.x); }
	double height() const { return std::abs(end.y - start.y); }
	double length() const { return start.distance_to(end); }

	void draw(cairo_t* cr) const {
		color.apply_to_cairo(cr);
		cairo_set_line_width(cr, 1.5);

		cairo_move_to(cr, start.x, start.y);
		cairo_line_to(cr, end.x, end.y);
		cairo_stroke(cr);

		if (grid.show_bounding_box) {
			double dashes[] = {4.0, 4.0};
			cairo_set_dash(cr, dashes, 2, 0);
			cairo_set_line_width(cr, 1.0);

			cairo_rectangle(cr, start.x, start.y, end.x - start.x, end.y - start.y);
			cairo_stroke(cr);

			cairo_set_dash(cr, nullptr, 0, 0);
		}
	}
};

class Cursor {
public:
	Point pos;
	Color color{0.0, 0.0, 0.0, 1.0};
	bool is_visible{true};

	void update(double x, double y) {
		pos.x = x;
		pos.y = y;
	}

	void draw(cairo_t* cr, double screen_w, double screen_h) const {
		if (!is_visible) return;

		color.apply_to_cairo(cr);
		cairo_set_line_width(cr, 1.0);

		cairo_move_to(cr, 0, pos.y);
		cairo_line_to(cr, screen_w, pos.y);

		cairo_move_to(cr, pos.x, 0);
		cairo_line_to(cr, pos.x, screen_h);

		cairo_stroke(cr);
	}
};

struct AppState {
	Cursor cursor;
	std::vector<Measurement> frozen_measurements;
	std::unique_ptr<Measurement> active_measurement;
};
