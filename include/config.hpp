#pragma once

#include "types.hpp"
#include <vector>
#include <string>

struct ColorScheme {
	Color main{0.0, 0.5, 1.0, 0.5};
	Color basic{0.0, 0.0, 0.0, 0.25};
	Color guide{0.45, 0.72, 0.0, 0.33};
	Color text_main{0.0, 0.5, 1.0, 0.6};
	Color text_basic{0.0, 0.0, 0.0, 0.75};
	Color text_faded{0.0, 0.0, 0.0, 0.4};
};

struct Keybindings {
	std::string quit{"Escape"};
	std::string fixed_ratio{"Shift_L"};
	std::string from_center{"Control_L"};
	std::string freeze{"f"};
	std::string guide_horizontal{"bracketleft"};
	std::string guide_vertical{"bracketright"};
	std::string snap_to_guides{"backslash"};
	std::string clear{"Delete"};
	std::string clear_last{"BackSpace"};
	std::string clear_all{"j"};
	std::string copy_values{"space"};
	std::string screenshot{"s"};
	std::string segment_line{"z"};
	std::string only_mask{"question"};

	std::string ratio_1_1{"q"};
	std::string ratio_4_3{"w"};
	std::string ratio_golden{"e"};
	std::string ratio_3_2{"r"};
	std::string ratio_16_9{"t"};
	std::string ratio_a4{"y"};
};

struct SelectionGuideRule {
	std::vector<double> x;
	std::vector<double> y;
	bool show{false};
};

struct SelectionGuides {
	SelectionGuideRule x{{0.5}, {0.5}, true};
	SelectionGuideRule c{{0.38197, 0.61803}, {0.38197, 0.61803}, false};
	SelectionGuideRule v{{0.33333, 0.66667}, {0.33333, 0.66667}, false};
	SelectionGuideRule b{{0.2, 0.4, 0.6, 0.8}, {0.2, 0.4, 0.6, 0.8}, false};
};

class Config {
public:
	std::string selection_mode{"inner"};
	int snap_distance{10};

	struct ScreenshotConfig {
		std::string target{"both"};
		std::string format{"png"};
		std::string file_pattern{"~/screenshot_%Y_%m_%d_%H_%M_%S.png"};
	} screenshot;

	Keybindings keys;
	SelectionGuides guides;

	ColorScheme theme_default{
		{0.0, 0.5, 1.0, 0.5},
		{0.0, 0.0, 0.0, 0.25},
		{0.45, 0.72, 0.0, 0.33},
		{0.0, 0.5, 1.0, 0.6},
		{0.0, 0.0, 0.0, 0.75},
		{0.0, 0.0, 0.0, 0.4}
	};

	ColorScheme theme_dark{
		{0.0, 0.0, 0.0, 0.8},
		{0.0, 0.0, 0.0, 0.5},
		{0.0, 0.0, 0.0, 0.4},
		{0.0, 0.0, 0.0, 0.9},
		{0.0, 0.0, 0.0, 0.75},
		{0.0, 0.0, 0.0, 0.5}
	};

	ColorScheme theme_light{
		{1.0, 1.0, 1.0, 0.8},
		{1.0, 1.0, 1.0, 0.5},
		{1.0, 1.0, 1.0, 0.4},
		{1.0, 1.0, 1.0, 0.9},
		{1.0, 1.0, 1.0, 0.75},
		{1.0, 1.0, 1.0, 0.5}
	};

	ColorScheme theme_highlight{
		{1.0, 0.23, 0.0, 0.8},
		{1.0, 0.23, 0.0, 0.5},
		{1.0, 0.23, 0.0, 0.4},
		{1.0, 0.23, 0.0, 0.9},
		{1.0, 0.23, 0.0, 0.75},
		{1.0, 0.23, 0.0, 0.5}
	};
	ColorScheme current_theme = theme_default;

	bool load_from_file(const std::string& filepath);
};
