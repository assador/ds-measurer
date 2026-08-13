#pragma once

#include <cstdint>
#include <unordered_map>
#include "core/types.hpp"

using Keybindings = std::unordered_map<std::string, uint32_t>;
using AspectRatios = std::unordered_map<uint32_t, double>;
using SelectionGuides = std::unordered_map<uint32_t, SelectionGuideRule>;
using ColorSchemes = std::unordered_map<std::string, ColorScheme>;
using ThemeNames = std::unordered_map<uint32_t, std::string>;

struct ScreenshotConfig {
	std::string target{"both"};
	std::string format{"png"};
	std::string file_pattern{"~/screenshot_%Y_%m_%d_%H_%M_%S.png"};
};

std::string resolve_config_path();

class Config {
public:
	std::string copy_format{"{w} × {h}"};
	bool show_diagonal{true};
	int snap_distance{10};
	TextStyle text_style = {.font_family = "Sans", .font_size = 10.0};

	Keybindings keys;
	AspectRatios ratios;
	SelectionGuides guides;
	ColorSchemes color_schemes;
	ThemeNames theme_names;

	ScreenshotConfig screenshot;

	bool load_from_file(const std::string& filepath);
};
