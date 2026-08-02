#include "config/config.hpp"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <algorithm>

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
			for (const auto& node : doc["shortcuts"]) {
				std::string key = node.first.as<std::string>();
				std::replace(key.begin(), key.end(), ' ', '_');
				keys[key] = node.second.as<std::string>();
			}
		}
		if (doc["selection guides"]) {
			for (const auto& node : doc["selection guides"]) {
				std::string key = node.first.as<std::string>();
				const auto& val = node.second;
				SelectionGuideRule rule;
				if (val["x"]) rule.x = val["x"].as<std::vector<double>>();
				if (val["y"]) rule.y = val["y"].as<std::vector<double>>();
				if (val["show"]) rule.show = val["show"].as<bool>();
				guides[key] = rule;
			}
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
			for (const auto& node : doc["colors"]) {
				std::string key = node.first.as<std::string>();
				ColorScheme scheme;
				parse_theme(node.second, scheme);
				themes[key] = scheme;
			}
		}

		if (auto it = themes.find("default"); it != themes.end()) current_theme = &it->second;
		else if (!themes.empty()) current_theme = &themes.begin()->second;

		if (doc["colors shortcuts"]) {
			for (const auto& node : doc["colors shortcuts"]) {
				std::string key = node.first.as<std::string>();
				std::string val = node.second.as<std::string>();
				keys["theme_" + key] = val;
			}
		}

		return true;
	} catch (const std::exception& e) {
		std::cerr << "Failed to load config " << filepath << ": " << e.what() << "\n";
		std::cerr << "We use built-in defaults.\n";
		return false;
	}
}
