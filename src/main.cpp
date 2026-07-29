#include "config.hpp"
#include "app_state.hpp"
#include "window.hpp"
#include "shortcuts.hpp"
#include "shobzaebis.hpp"
#include <gtk/gtk.h>
#include <filesystem>
#include <cstdlib>

static Config config;
static AppState state(config);

// SEC Config

namespace fs = std::filesystem;

std::string resolve_config_path() {
	if (const char* home = std::getenv("HOME")) {
		fs::path xdg_path = fs::path(home) / ".config" / "ds-measurer" / "ds-measurer.yaml";
		if (fs::exists(xdg_path)) return xdg_path.string();
		fs::path home_path = fs::path(home) / ".ds-measurer.yaml";
		if (fs::exists(home_path)) return home_path.string();
	}
	return "ds-measurer.yaml";
}

// SEC Main

int main(int argc, char** argv) {
	config.load_from_file(resolve_config_path());
	state.cursor.color = config.current_theme.basic;
	ShortcutManager::init(config.keys);

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
