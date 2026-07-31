#include "config.hpp"
#include <yaml-cpp/yaml.h>
#include <iostream>

bool Config::load_from_file(const std::string& filepath) {
	try {
		YAML::Node doc = YAML::LoadFile(filepath);

		if (doc["common"]["selection mode"]) {
			selection_mode = doc["common"]["selection mode"].as<std::string>();
		}
		if (doc["guides"]["snap distance"]) {
			snap_distance = doc["guides"]["snap distance"].as<int>();
		}
		if (doc["shortcuts"]) {
			auto sc = doc["shortcuts"];
			if (sc["quit"]) keys.quit = sc["quit"].as<std::string>();
			if (sc["freeze"]) keys.freeze = sc["freeze"].as<std::string>();
			if (sc["clear"]) keys.clear = sc["clear"].as<std::string>();
			if (sc["clear last"]) keys.clear_last = sc["clear last"].as<std::string>();
			if (sc["clear all"]) keys.clear_all = sc["clear all"].as<std::string>();
			if (sc["copy values"]) keys.copy_values = sc["copy values"].as<std::string>();
			if (sc["screenshot"]) keys.screenshot = sc["screenshot"].as<std::string>();
		}

		auto parse_color = [](const YAML::Node& node) -> Color {
			return Color{
				node["r"].as<double>(),
				node["g"].as<double>(),
				node["b"].as<double>(),
				node["a"].as<double>()
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
			auto colors = doc["colors"];
			parse_theme(colors["default"], theme_default);
			parse_theme(colors["dark"], theme_dark);
			parse_theme(colors["white"], theme_white);
			parse_theme(colors["black"], theme_black);
		}
		current_theme = theme_default;
		return true;

	} catch (const std::exception& e) {
		std::cerr << "Failed to load config " << filepath << ": " << e.what() << "\n";
		std::cerr << "We use built-in defaults.\n";
		return false;
	}
}
