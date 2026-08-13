#include "config/config.hpp"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <gtk/gtk.h>
#include <yaml-cpp/yaml.h>
#include "vendor/tinyexpr.h"

namespace fs = std::filesystem;

fs::path get_executable_dir() {
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	if (count != -1) return fs::path(std::string(result, count)).parent_path();
	return fs::current_path();
}

std::string resolve_config_path() {
	if (const char* home = std::getenv("HOME")) {
		fs::path xdg_path = fs::path(home) / ".config" / "ds-measurer" / "ds-measurer.yaml";
		if (fs::exists(xdg_path)) return xdg_path.string();
		fs::path home_path = fs::path(home) / ".ds-measurer.yaml";
		if (fs::exists(home_path)) return home_path.string();
	}
	if (fs::exists("/etc/ds-measurer/ds-measurer.yaml")) {
		return "/etc/ds-measurer/ds-measurer.yaml";
	}
	fs::path exe_dir_config = get_executable_dir() / "ds-measurer.yaml";
	if (fs::exists(exe_dir_config)) return exe_dir_config.string();
	return "ds-measurer.yaml";
}

double parse_math_expression(const std::string& expr_str, double fallback = 1.0) {
	int error = 0;
	double result = te_interp(expr_str.c_str(), &error);
	if (error != 0) {
		std::cerr
			<< "Failed to parse math expression '"
			<< expr_str << "' (error at char " << error
			<< "). Using fallback: " << fallback << "\n"
		;
		return fallback;
	}
	return result;
}

static uint32_t key_name_to_keycode(const std::string& name) {
	guint kv = gdk_keyval_from_name(name.c_str());
	if (kv == GDK_KEY_VoidSymbol) return 0;

	GdkDisplay* display = gdk_display_get_default();
	if (!display) return 0;

	GdkKeymapKey* keys_array = nullptr;
	int n_keys = 0;
	uint32_t keycode = 0;

	if (gdk_display_map_keyval(display, kv, &keys_array, &n_keys) && n_keys > 0) {
		keycode = static_cast<uint32_t>(keys_array[0].keycode);
		g_free(keys_array);
	}
	return keycode;
}

bool Config::load_from_file(const std::string& filepath) {
	try {
		YAML::Node doc = YAML::LoadFile(filepath);

		if (doc["common"]["copy format"]) {
			copy_format = doc["common"]["copy format"].as<std::string>();
		}
		if (doc["common"]["font family"]) {
			text_style.font_family = doc["common"]["font family"].as<std::string>();
		}
		if (doc["common"]["font size"]) {
			text_style.font_size = doc["common"]["font size"].as<double>();
		}
		if (doc["common"]["show diagonal"]) {
			show_diagonal = doc["common"]["show diagonal"].as<bool>();
		}
		if (doc["guides"]["snap distance"]) {
			snap_distance = doc["guides"]["snap distance"].as<int>();
		}
		if (doc["shortcuts"]) {
			for (const auto& node : doc["shortcuts"]) {
				auto key = node.first.as<std::string>();
				std::ranges::replace(key, ' ', '_');
				guint kc = key_name_to_keycode(node.second.as<std::string>());
				if (kc != 0) keys[key] = kc;
			}
		}
		if (doc["aspect ratio shortcuts"]) {
			for (const auto& node : doc["aspect ratio shortcuts"]) {
				guint kc = key_name_to_keycode(node.first.as<std::string>());
				if (kc != 0) ratios[kc] = parse_math_expression(node.second.as<std::string>());
			}
		}
		if (doc["selection guides"]) {
			for (const auto& node : doc["selection guides"]) {
				guint kc = key_name_to_keycode(node.first.as<std::string>());
				if (kc != 0) {
					const auto& val = node.second;
					SelectionGuideRule rule;
					if (val["x"]) rule.x = val["x"].as<std::vector<double>>();
					if (val["y"]) rule.y = val["y"].as<std::vector<double>>();
					if (val["show"]) rule.show = val["show"].as<bool>();
					guides[kc] = rule;
				}
			}
		}
		if (doc["colors shortcuts"]) {
			for (const auto& node : doc["colors shortcuts"]) {
				guint kc = key_name_to_keycode(node.second.as<std::string>());
				if (kc != 0) theme_names[kc] = node.first.as<std::string>();
			}
		}
		auto parse_color = [](const YAML::Node& node) -> Color {
			return Color{
				.r=node["r"].as<double>(),
				.g=node["g"].as<double>(),
				.b=node["b"].as<double>(),
				.a=node["a"].as<double>()
			};
		};
		auto parse_theme = [&](const YAML::Node& node, ColorScheme& theme) {
			if (!node) return;
			if (node["main"]) theme.main = parse_color(node["main"]);
			if (node["basic"]) theme.basic = parse_color(node["basic"]);
			if (node["guide"]) theme.guide = parse_color(node["guide"]);
			if (node["text main"]) theme.text_main = parse_color(node["text main"]);
			if (node["text basic"]) theme.text_basic = parse_color(node["text basic"]);
			if (node["text faded"]) theme.text_faded = parse_color(node["text faded"]);
		};
		if (doc["colors"]) {
			for (const auto& node : doc["colors"]) {
				auto key = node.first.as<std::string>();
				ColorScheme scheme;
				parse_theme(node.second, scheme);
				color_schemes[key] = scheme;
			}
		}

		return true;
	} catch (const std::exception& e) {
		std::cerr << "Failed to load config " << filepath << ": " << e.what() << "\n";
		std::cerr << "We use built-in defaults.\n";
		return false;
	}
}
