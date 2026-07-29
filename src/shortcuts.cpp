#include "shortcuts.hpp"
#include <functional>
#include <vector>
#include <gtk/gtk.h>

struct ActionBinding {
	guint keyval{0};
	std::function<void(GtkWidget*, GtkApplication*, AppState&)> action;
};

static std::vector<ActionBinding> g_bindings;

void ShortcutManager::init(const Keybindings& keys) {
	g_bindings.clear();

	auto add = [](const std::string& key_name, auto action) {
		guint kv = gdk_keyval_from_name(key_name.c_str());
		if (kv != GDK_KEY_VoidSymbol) g_bindings.push_back({kv, action});
	};
	add(keys.quit, [](GtkWidget*, GtkApplication* app, AppState&) {
		g_application_quit(G_APPLICATION(app));
	});
	add(keys.freeze, [](GtkWidget* widget, GtkApplication*, AppState& state) {
		if (state.active_measurement) {
			state.active_measurement->is_frozen = true;
			state.frozen_measurements.push_back(
				std::move(*state.active_measurement)
			);
			state.active_measurement.reset();
			gtk_widget_queue_draw(widget);
		}
	});
	add(keys.clear_last,
		[](GtkWidget* widget, GtkApplication*, AppState& state) {
			if (!state.frozen_measurements.empty()) {
				state.frozen_measurements.pop_back();
				gtk_widget_queue_draw(widget);
			}
		});
	auto clear_all = [](GtkWidget* widget, GtkApplication*, AppState& state) {
		state.frozen_measurements.clear();
		state.active_measurement.reset();
		gtk_widget_queue_draw(widget);
	};
	add(keys.clear, clear_all);
	add(keys.clear_all, clear_all);
}

gboolean ShortcutManager::handle_key(
	guint keyval,
	GtkWidget* widget,
	GtkApplication* app,
	AppState& state
) {
	for (const auto& binding : g_bindings) {
		if (binding.keyval == keyval) {
			binding.action(widget, app, state);
			return TRUE;
		}
	}
	return FALSE;
}
