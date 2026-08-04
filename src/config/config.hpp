#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include "core/types.hpp"

struct ColorScheme {
	Color main{.r=0.0, .g=0.5, .b=1.0, .a=0.5};
	Color basic{.r=0.0, .g=0.0, .b=0.0, .a=0.25};
	Color guide{.r=0.45, .g=0.72, .b=0.0, .a=0.33};
	Color text_main{.r=0.0, .g=0.5, .b=1.0, .a=0.6};
	Color text_basic{.r=0.0, .g=0.0, .b=0.0, .a=0.75};
	Color text_faded{.r=0.0, .g=0.0, .b=0.0, .a=0.4};
};
struct SelectionGuideRule {
	std::vector<double> x;
	std::vector<double> y;
	bool show{false};
};

using Keybindings = std::unordered_map<std::string, uint32_t>;
using AspectRatios = std::unordered_map<uint32_t, double>;
using SelectionGuides = std::unordered_map<uint32_t, SelectionGuideRule>;
using ColorSchemes = std::unordered_map<std::string, ColorScheme>;
using ThemeNames = std::unordered_map<uint32_t, std::string>;

std::string resolve_config_path();

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
	SelectionGuides guides;
	ColorSchemes color_schemes;
	ThemeNames theme_names;

	ColorScheme* current_color_scheme = nullptr;

	bool load_from_file(const std::string& filepath);
};
