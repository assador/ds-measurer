#include "core/app.hpp"
#include "config/config.hpp"

static Config config;
static AppState state(config);

int main(int argc, char** argv) {
	setenv("GDK_BACKEND", "wayland", 1);
	return run_application(argc, argv, state, config);
}
