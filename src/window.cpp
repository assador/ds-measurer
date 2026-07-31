#include "window.hpp"
#include "shortcuts.hpp"
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

	if (state->active_measurement) {
		if (state->active_measurement->is_moving) {
			int dx = static_cast<int>(x) - state->rmb.click_pos.x;
			int dy = static_cast<int>(y) - state->rmb.click_pos.y;

			state->active_measurement->start = {
				state->rmb.initial_p1.x + dx,
				state->rmb.initial_p1.y + dy,
			};
			state->active_measurement->end = {
				state->rmb.initial_p2.x + dx,
				state->rmb.initial_p2.y + dy,
			};
		}
		else if (state->active_measurement->is_changing) {
			state->active_measurement->end = Point{x, y};
		}
	}

	gtk_widget_queue_draw(widget);
}

static gboolean on_legacy_event(
	GtkEventControllerLegacy* controller,
	GdkEvent* event,
	gpointer user_data
) {
	auto* state = static_cast<AppState*>(user_data);
	auto* widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller));
	GdkEventType type = gdk_event_get_event_type(event);

	if (type == GDK_BUTTON_PRESS) {
		guint button = gdk_button_event_get_button(event);
		double x = 0, y = 0;
		gdk_event_get_position(event, &x, &y);

		if (button == GDK_BUTTON_PRIMARY) {
			state->lmb.is_dragging = true;
			state->lmb.click_pos = Point{static_cast<int>(x), static_cast<int>(y)};

			if (!state->active_measurement) {
				state->active_measurement = std::make_unique<Measurement>(
					Point{x, y},
					Point{x, y},
					state->config.guides
				);
			}
			state->active_measurement->is_changing = true;

			gtk_widget_queue_draw(widget);
			return GDK_EVENT_STOP;
		}
		else if (button == GDK_BUTTON_SECONDARY) {
			state->rmb.is_dragging = true;
			state->rmb.click_pos = Point{static_cast<int>(x), static_cast<int>(y)};

			if (state->active_measurement) {
				state->active_measurement->is_moving = true;
				state->rmb.initial_p1 = state->active_measurement->start;
				state->rmb.initial_p2 = state->active_measurement->end;
			}

			gtk_widget_queue_draw(widget);
			return GDK_EVENT_STOP;
		}
	}
	else if (type == GDK_BUTTON_RELEASE) {
		guint button = gdk_button_event_get_button(event);

		if (button == GDK_BUTTON_PRIMARY) {
			state->lmb.is_dragging = false;
			if (state->active_measurement) state->active_measurement->is_changing = false;
			gtk_widget_queue_draw(widget);
			return GDK_EVENT_STOP;
		}
		else if (button == GDK_BUTTON_SECONDARY) {
			state->rmb.is_dragging = false;
			if (state->active_measurement) state->active_measurement->is_moving = false;
			gtk_widget_queue_draw(widget);
			return GDK_EVENT_STOP;
		}
	}

	return GDK_EVENT_PROPAGATE;
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
	state->cursor.draw(cr, width, height, state->config.current_theme.basic);
}

static gboolean on_key_pressed(
	GtkEventControllerKey*,
	guint keyval,
	guint,
	GdkModifierType,
	gpointer user_data
) {
	auto* state_ptr = static_cast<AppState*>(user_data);
	return ShortcutManager::handle_key(keyval, *state_ptr);
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

	state.drawing_area = drawing_area;

	gtk_window_set_child(GTK_WINDOW(window), drawing_area);
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), on_draw, &state, nullptr);

	GdkCursor* invisible_cursor = create_invisible_cursor(display);
	gtk_widget_set_cursor(drawing_area, invisible_cursor);
	g_object_unref(invisible_cursor);

	GtkEventController* motion_controller = gtk_event_controller_motion_new();
	g_signal_connect(motion_controller, "motion", G_CALLBACK(on_motion), &state);
	gtk_widget_add_controller(drawing_area, motion_controller);

	GtkEventController* legacy_controller = gtk_event_controller_legacy_new();
	g_signal_connect(legacy_controller, "event", G_CALLBACK(on_legacy_event), &state);
	gtk_widget_add_controller(drawing_area, legacy_controller);

	GtkEventController* key_controller = gtk_event_controller_key_new();
	g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), &state);
	gtk_widget_add_controller(window, key_controller);

	gtk_window_present(GTK_WINDOW(window));
}
