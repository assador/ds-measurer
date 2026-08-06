#include "core/app.hpp"
#include <gtk/gtk.h>
#include "core/shobzaebis.hpp"
#include "ui/shortcuts.hpp"
#include "ui/window.hpp"

void AppState::request_quit() {
	g_application_quit(g_application_get_default());
}

int run_application(int argc, char** argv, AppState& state, Config& config) {
	GtkApplication* app = gtk_application_new(
		"org.assador.ds.Measurer",
		G_APPLICATION_DEFAULT_FLAGS
	);
	g_signal_connect(
		app,
		"activate",
		G_CALLBACK(+[](GtkApplication* app, gpointer user_data) {
			auto* state = static_cast<AppState*>(user_data);
			state->config.load_from_file(resolve_config_path());
			state->apply_from_config();
			ShortcutManager::init(state->config);
			setup_main_window(app, *state, state->config);
		}),
		&state
	);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	shobzaebis::do_everything();
	g_object_unref(app);
	return status;
}
