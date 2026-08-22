#include "core/app.hpp"
#include "config/config.hpp"

static Config config;
static AppState state(config);

int main(int argc, char** argv) {
	return run_application(argc, argv, state, config);
}
