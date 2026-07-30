#include "window.hpp"
#include "shortcuts.hpp"
// #include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

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
	auto* state = static_cast<AppState*>(user_data);
	auto* widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller));

	state->cursor.update(static_cast<int>(x), static_cast<int>(y));
	gtk_widget_queue_draw(widget);
}

static void on_draw(
	GtkDrawingArea* area,
	cairo_t* cr,
	int width,
	int height,
	gpointer user_data
) {
	auto* state = static_cast<AppState*>(user_data);

	cairo_save(cr);
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	cairo_restore(cr);

	for (const auto& m : state->frozen_measurements) {
		m.draw(cr, state->config.current_theme);
	}
	if (state->active_measurement) {
		state->active_measurement->draw(cr, state->config.current_theme);
	}
	state->cursor.draw(cr, static_cast<double>(width), static_cast<double>(height));
}

static void on_drag_begin(
	GtkGestureDrag* gesture,
	double start_x,
	double start_y,
	gpointer user_data
) {
	auto* state = static_cast<AppState*>(user_data);
	auto* widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));

	state->active_measurement =
		std::make_unique<Measurement>(
			Point{start_x, start_y},
			Point{start_x, start_y},
			state->config.guides
		)
	;
	state->cursor.update(start_x, start_y);
	gtk_widget_queue_draw(widget);
}

static void on_drag_update(
	GtkGestureDrag* gesture,
	double offset_x,
	double offset_y,
	gpointer user_data
) {
	auto* state = static_cast<AppState*>(user_data);
	auto* widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));

	if (state->active_measurement) {
		double start_x, start_y;
		gtk_gesture_drag_get_start_point(gesture, &start_x, &start_y);
		double curr_x = start_x + offset_x;
		double curr_y = start_y + offset_y;

		state->active_measurement->end = Point{curr_x, curr_y};
		state->cursor.update(curr_x, curr_y);
	}
	gtk_widget_queue_draw(widget);
}

static void on_drag_end(
	GtkGestureDrag* gesture,
	double offset_x,
	double offset_y,
	gpointer user_data
) {
	auto* state = static_cast<AppState*>(user_data);
	auto* widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));

	state->active_measurement.reset();
	gtk_widget_queue_draw(widget);
}

static gboolean on_key_pressed(
	GtkEventControllerKey* controller,
	guint keyval,
	guint keycode,
	GdkModifierType state,
	gpointer user_data
) {
	auto* state_ptr = static_cast<AppState*>(user_data);
	auto* app = GTK_APPLICATION(g_object_get_data(G_OBJECT(controller), "app"));
	auto* widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller));

	return ShortcutManager::handle_key(keyval, widget, app, *state_ptr);
}

void setup_main_window(
	GtkApplication* app,
	AppState& state,
	const Config& config
) {
	GtkWidget* window = gtk_application_window_new(app);

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
	gtk_layer_set_keyboard_mode(GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

	GdkDisplay* display = gdk_display_get_default();
	GtkWidget* drawing_area = gtk_drawing_area_new();
	gtk_window_set_child(GTK_WINDOW(window), drawing_area);
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), on_draw, &state, nullptr);

	GdkCursor* invisible_cursor = create_invisible_cursor(display);
	gtk_widget_set_cursor(drawing_area, invisible_cursor);
	g_object_unref(invisible_cursor);

	GtkEventController* motion_controller = gtk_event_controller_motion_new();
	g_signal_connect(motion_controller, "motion", G_CALLBACK(on_motion), &state);
	gtk_widget_add_controller(drawing_area, motion_controller);

	GtkGesture* drag_gesture = gtk_gesture_drag_new();
	g_signal_connect(drag_gesture, "drag-begin", G_CALLBACK(on_drag_begin), &state);
	g_signal_connect(drag_gesture, "drag-update", G_CALLBACK(on_drag_update), &state);
	g_signal_connect(drag_gesture, "drag-end", G_CALLBACK(on_drag_end), &state);
	gtk_widget_add_controller(drawing_area, GTK_EVENT_CONTROLLER(drag_gesture));

	GtkEventController* key_controller = gtk_event_controller_key_new();
	g_object_set_data(G_OBJECT(key_controller), "app", app);
	g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), &state);
	gtk_widget_add_controller(window, key_controller);

	gtk_window_present(GTK_WINDOW(window));
}
