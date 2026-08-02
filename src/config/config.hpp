#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "core/types.hpp"

struct ColorScheme {
	Color main{0.0, 0.5, 1.0, 0.5};
	Color basic{0.0, 0.0, 0.0, 0.25};
	Color guide{0.45, 0.72, 0.0, 0.33};
	Color text_main{0.0, 0.5, 1.0, 0.6};
	Color text_basic{0.0, 0.0, 0.0, 0.75};
	Color text_faded{0.0, 0.0, 0.0, 0.4};
};
struct SelectionGuideRule {
	std::vector<double> x;
	std::vector<double> y;
	bool show{false};
};

using Keybindings = std::unordered_map<std::string, std::string>;
using AspectRatios = std::unordered_map<std::string, double>;
using ColorSchemes = std::unordered_map<std::string, ColorScheme>;
using SelectionGuides = std::unordered_map<std::string, SelectionGuideRule>;

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
	AspectRatios ratios;
	ColorSchemes themes;
	SelectionGuides guides;

	ColorScheme* current_theme = nullptr;

	bool load_from_file(const std::string& filepath);
};
