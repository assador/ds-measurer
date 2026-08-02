#include "core/app_state.hpp"
#include "core/shobzaebis.hpp"
#include "config/config.hpp"
#include "ui/window.hpp"
#include "ui/shortcuts.hpp"
#include <gtk/gtk.h>
#include <filesystem>
#include <cstdlib>
#include <ranges>
#include <unistd.h>

static Config config;
static AppState state(config);

// SEC Config

#include <filesystem>

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

// SEC Main

int main(int argc, char** argv) {
	config.load_from_file(resolve_config_path());

	auto guide_keys = config.guides | std::views::keys;
	ShortcutManager::init(config.keys, {guide_keys.begin(), guide_keys.end()});

	GtkApplication* app = gtk_application_new(
		"org.assador.ds.Measurer",
		G_APPLICATION_DEFAULT_FLAGS
	);

	g_signal_connect(
		app,
		"activate",
		G_CALLBACK(+[](GtkApplication* app, gpointer) {
			setup_main_window(app, state, config);
		}),
		nullptr
	);

	int status = g_application_run(G_APPLICATION(app), argc, argv);
	shobzaebis::do_everything();
	g_object_unref(app);
	return status;
}
