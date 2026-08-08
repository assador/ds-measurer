#include "core/measurement.hpp"
#include <format>

Measurement::Measurement(
	Point p1,
	Point p2,
	MeasurementOptions opts,
	const SelectionGuides& guides
) :
	start(p1),
	end(p2),
	show_diagonal(opts.show_diagonal)
{
	grids.reserve(guides.size());
	for (const auto& [key, rule] : guides) {
		grids.emplace(key, Grid{rule});
	}
}

void Measurement::toggle_grid(uint32_t kc) {
	auto it = grids.find(kc);
	if (it == grids.end()) return;
	it->second.rule.show = !it->second.rule.show;
}

void Measurement::move_by(int dx, int dy) {
	start.x += dx;
	start.y += dy;
	end.x += dx;
	end.y += dy;
}
void Measurement::move_by(const Point& delta) { move_by(delta.x, delta.y); }

void Measurement::move_from(const Point& p1, const Point& p2, int dx, int dy) {
	start = Point{p1.x + dx, p1.y + dy};
	end = Point{p2.x + dx, p2.y + dy};
}

void Measurement::apply_modifiers(
	const Point& i_start,
	const Point& i_end,
	bool from_center,
	bool fixed_ratio,
	double ratio
) {
	if (fixed_ratio) {
		double dx = i_end.x - i_start.x;
		double dy = i_end.y - i_start.y;
		double abs_dx = std::abs(dx);
		double abs_dy = std::abs(dy);

		if (abs_dx >= abs_dy) {
			end.x = i_start.x + static_cast<int>(std::copysign(abs_dy * ratio, dx));
			end.y = i_end.y;
		} else {
			end.x = i_end.x;
			end.y = i_start.y + static_cast<int>(std::copysign(abs_dx / ratio, dy));
		}
	} else {
		end = i_end;
	}
	if (from_center) {
		// with the end already calculated 
		start.x = i_start.x * 2 - end.x;
		start.y = i_start.y * 2 - end.y;
	} else {
		start = i_start;
	}
}

std::string Measurement::values_string(std::string_view fmt) const {
	std::string result(fmt);

	auto replace = [&result](std::string_view tag, const auto& val) {
		std::string tag_str(tag);
		size_t pos = 0;
		std::string val_str;
		if constexpr (std::is_floating_point_v<std::decay_t<decltype(val)>>) {
			val_str = std::format("{:.6g}", val);
		} else {
			val_str = std::to_string(val);
		}
		while ((pos = result.find(tag_str, pos)) != std::string::npos) {
			result.replace(pos, tag_str.length(), val_str);
			pos += val_str.length();
		}
	};
	replace("{w}", width(true));
	replace("{h}", height(true));
	replace("{l}", length(true));
	replace("{a}", angle_deg(true));
	replace("{x1}", start.x + (end.x < start.x ? 1 : 0));
	replace("{y1}", start.y + (end.y < start.y ? 1 : 0));
	replace("{x2}", end.x + (end.x < start.x ? 0 : 1));
	replace("{y2}", end.y + (end.y < start.y ? 0 : 1));

	return result;
}
