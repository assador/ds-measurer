#pragma once

#include <format>
#include <algorithm>
#include <cmath>
#include <format>
#include <string>
#include "core/types.hpp"

namespace utils {

inline std::string color_to_formats_str(const Color& color) {
	ColorHSLA hsl = rgb_to_hsl(color.r, color.g, color.b);

	auto to_byte = [](double v) {
		return std::clamp(static_cast<int>(std::round(v * 255.0)), 0, 255);
	};
	auto fmt_num = [](double v) {
		std::string s = std::format("{:.3f}", v);
		if (s.find('.') != std::string::npos) {
			s.erase(s.find_last_not_of('0') + 1, std::string::npos);
			if (s.back() == '.') s.pop_back();
		}
		return s;
	};

	double r_b = color.r * 255.0;
	double g_b = color.g * 255.0;
	double b_b = color.b * 255.0;

	std::string hex_str = std::format(
		"#{:02x}{:02x}{:02x}",
		to_byte(color.r),
		to_byte(color.g),
		to_byte(color.b)
	);
	std::string rgb_bytes_str =
		std::format("rgb({} {} {})",
		fmt_num(r_b),
		fmt_num(g_b),
		fmt_num(b_b)
	);
	std::string rgb_percentages_str = std::format(
		"rgb({}% {}% {}%)",
		fmt_num(color.r * 100.0),
		fmt_num(color.g * 100.0),
		fmt_num(color.b * 100.0)
	);
	std::string hsl_str = std::format(
		"hsl({} {}% {}%)",
		fmt_num(hsl.h),
		fmt_num(hsl.s * 100.0),
		fmt_num(hsl.l * 100.0)
	);

	return std::format("{}\n{}\n{}\n{}", hex_str, rgb_bytes_str, rgb_percentages_str, hsl_str);
}

} // namespace utils
