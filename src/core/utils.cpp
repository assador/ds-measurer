#include "utils.hpp"
#include <cstdlib>
#include <ctime>
#include <vector>

namespace utils {

std::string expand_path_pattern(const std::string& pattern) {
	std::string path_str = pattern;

	if (!path_str.empty() && path_str[0] == '~') {
		const char* home = std::getenv("HOME");
		if (home) path_str.replace(0, 1, home);
	}

	std::time_t t = std::time(nullptr);
	std::tm tm = *std::localtime(&t);

	std::vector<char> buffer(512);
	size_t written = std::strftime(buffer.data(), buffer.size(), path_str.c_str(), &tm);
	if (written > 0) path_str = std::string(buffer.data(), written);

	return path_str;
}

} // namespace utils
