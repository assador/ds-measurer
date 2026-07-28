#include "config.hpp"
#include "app_state.hpp"
#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <filesystem>
#include <cstdlib>

static AppState g_state;
static Config g_config;

static void on_draw(
	GtkDrawingArea* area,
	cairo_t* cr,
	int width,
	int height,
	gpointer user_data
) {
	cairo_save(cr);
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	cairo_restore(cr);

	for (const auto& m : g_state.frozen_measurements) {
		m.draw(cr);
	}
	if (g_state.active_measurement) {
		g_state.active_measurement->draw(cr);
	}
	g_state.cursor.draw(cr, static_cast<double>(width), static_cast<double>(height));
}

// SEC Mouse

static GdkCursor* create_invisible_cursor(GdkDisplay* display) {
	guchar* pixels = static_cast<guchar*>(g_malloc0(4));
	GBytes* bytes = g_bytes_new_take(pixels, 4);
	GdkTexture* texture = gdk_memory_texture_new(1, 1, GDK_MEMORY_R8G8B8A8_PREMULTIPLIED, bytes, 4);
	g_bytes_unref(bytes);
	GdkCursor* cursor = gdk_cursor_new_from_texture(texture, 0, 0, nullptr);
	g_object_unref(texture);
	return cursor;
}

static void on_motion(
	GtkEventControllerMotion* controller,
	double x,
	double y,
	gpointer user_data
) {
	auto* drawing_area = GTK_WIDGET(user_data);

	g_state.cursor.update(x, y);

	if (g_state.active_measurement) {
		g_state.active_measurement->end = g_state.cursor.pos;
	}
	gtk_widget_queue_draw(drawing_area);
}

// SEC Keys

static gboolean on_key_pressed(
	GtkEventControllerKey* controller,
	guint keyval,
	guint keycode,
	GdkModifierType state,
	gpointer user_data
) {
	if (keyval == GDK_KEY_Escape) {
		auto* window = GTK_WIDGET(user_data);
		auto* app = G_APPLICATION(g_object_get_data(G_OBJECT(window), "app"));
		g_application_quit(app);
		return TRUE;
	}
	return FALSE;
}

// SEC GTK

static void on_activate(GtkApplication* app, gpointer user_data) {
	GtkWidget* window = gtk_application_window_new(app);
	g_object_set_data(G_OBJECT(window), "app", app);

	GtkCssProvider* provider = gtk_css_provider_new();
	gtk_css_provider_load_from_string(
		provider,
		"window, contents { background-color: transparent; }"
	);
	gtk_style_context_add_provider_for_display(
		gdk_display_get_default(),
		GTK_STYLE_PROVIDER(provider),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
	);
	g_object_unref(provider);

	gtk_layer_init_for_window(GTK_WINDOW(window));
	gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_OVERLAY);
	gtk_layer_set_keyboard_mode(
		GTK_WINDOW(window),
		GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE
	);

	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

	gtk_layer_set_exclusive_zone(GTK_WINDOW(window), -1);

	GdkDisplay* display = gdk_display_get_default();
	GdkCursor* invisible_cursor = create_invisible_cursor(display);
	gtk_widget_set_cursor(window, invisible_cursor);
	GtkWidget* drawing_area = gtk_drawing_area_new();
	gtk_widget_set_cursor(drawing_area, invisible_cursor);
	g_object_unref(invisible_cursor);

	gtk_drawing_area_set_draw_func(
		GTK_DRAWING_AREA(drawing_area),
		on_draw,
		nullptr,
		nullptr
	);
	gtk_window_set_child(GTK_WINDOW(window), drawing_area);

	GtkEventController* motion_controller = gtk_event_controller_motion_new();
	g_signal_connect(
		motion_controller,
		"motion",
		G_CALLBACK(on_motion),
		drawing_area
	);
	gtk_widget_add_controller(drawing_area, motion_controller);

	GtkGesture* click_gesture = gtk_gesture_click_new();
	gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click_gesture), 0);
	g_signal_connect(
		click_gesture,
		"pressed",
		G_CALLBACK(+[](GtkGestureClick* gesture, int n_press, double x, double y, gpointer user_data) {
			gtk_gesture_set_state(GTK_GESTURE(gesture), GTK_EVENT_SEQUENCE_CLAIMED);
		}),
		nullptr
	);
	gtk_widget_add_controller(drawing_area, GTK_EVENT_CONTROLLER(click_gesture));

	GtkEventController* key_controller = gtk_event_controller_key_new();
	g_signal_connect(
		key_controller,
		"key-pressed",
		G_CALLBACK(on_key_pressed),
		window
	);
	gtk_widget_add_controller(window, key_controller);

	gtk_window_present(GTK_WINDOW(window));
}

// SEC Config

namespace fs = std::filesystem;

std::string resolve_config_path() {
	if (const char* home = std::getenv("HOME")) {
		fs::path xdg_path = fs::path(home) / ".config" / "ds-measurer" / "ds-measurer.yaml";
		if (fs::exists(xdg_path)) {
			return xdg_path.string();
		}
		fs::path home_path = fs::path(home) / ".ds-measurer.yaml";
		if (fs::exists(home_path)) {
			return home_path.string();
		}
	}
	if (fs::exists("ds-measurer.yaml")) {
		return "ds-measurer.yaml";
	}
	return "ds-measurer.yaml";
}

// SEC Main

int main(int argc, char** argv) {
	g_config.load_from_file(resolve_config_path());
	g_state.cursor.color = g_config.current_theme.basic;

	GtkApplication* app =
		gtk_application_new("org.assador.ds.Measurer", G_APPLICATION_DEFAULT_FLAGS)
	;
	g_signal_connect(app, "activate", G_CALLBACK(on_activate), nullptr);

	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return status;
}
