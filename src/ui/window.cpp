#include "ui/window.hpp"
#include <filesystem>
#include <gtk4-layer-shell.h>
#include "core/color.hpp"
#include "core/types.hpp"
#include "core/utils.hpp"
#include "platform/screenshot.hpp"
#include "ui/shortcuts.hpp"
#include "ui/ui.hpp"

static void check_and_process(AppState* state) {
	if (state->lmb.is_dragging || state->rmb.is_dragging) return;
	if (state->draft_measurement) {
		if (state->active_measurement == state->draft_measurement.get()) {
			state->active_measurement = nullptr;
		}
		state->draft_measurement.reset();
	}
}

static bool texture_to_clipboard(GdkTexture* texture) {
	if (!texture) return false;
	GdkDisplay* display = gdk_display_get_default();
	GdkClipboard* clipboard = gdk_display_get_clipboard(display);
	gdk_clipboard_set_texture(clipboard, texture);
	return true;
}

static bool texture_to_file(GdkTexture* texture, const std::string& pattern) {
	if (!texture || pattern.empty()) return false;

	namespace fs = std::filesystem;
	fs::path full_path = utils::expand_path_pattern(pattern);

	std::error_code ec;
	if (full_path.has_parent_path()) fs::create_directories(full_path.parent_path(), ec);

	GError* error = nullptr;
	if (!gdk_texture_save_to_png(texture, full_path.string().c_str())) {
		if (error) {
			g_warning("Failed to save screenshot: %s", error->message);
			g_error_free(error);
		}
		return false;
	}
	return true;
}

static GdkCursor* create_invisible_cursor(GdkDisplay* display) {
	auto* pixels = static_cast<guchar*>(g_malloc0(4));
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

	if (
		state->active_measurement && (
			state->mode == Mode::Measurements ||
			state->active_measurement == state->draft_measurement.get()
		)
	) {
		auto& m = *state->active_measurement;
		if (state->rmb.is_dragging) {
			int dx = static_cast<int>(x) - state->rmb.click_pos.x;
			int dy = static_cast<int>(y) - state->rmb.click_pos.y;
			m.move_from(state->rmb.initial_p1, state->rmb.initial_p2, dx, dy);
			state->snap_active_translation();
		} else if (state->lmb.is_dragging) {
			Point target_pos = state->get_snapped_pos(
				Point{static_cast<int>(x), static_cast<int>(y)}
			);
			m.apply_modifiers(
				state->lmb.initial_p1,
				target_pos,
				state->is_from_center,
				state->is_fixed_ratio,
				state->ratio
			);
		}
	}
	if (state->rmb.is_dragging) {
		if (state->active_guide && state->mode == Mode::Guides) {
			auto& g = *state->active_guide;
			if (g.orientation == Orientation::Horizontal) {
				g.coord = state->rmb.initial_p0.y + static_cast<int>(y) - state->rmb.click_pos.y;
			} else if (g.orientation == Orientation::Vertical) {
				g.coord = state->rmb.initial_p0.x + static_cast<int>(x) - state->rmb.click_pos.x;
			}
		}
		if (state->active_color_pick && state->mode == Mode::ColorPicks) {
			auto& p = *state->active_color_pick;
			int dx = static_cast<int>(x) - state->rmb.click_pos.x;
			int dy = static_cast<int>(y) - state->rmb.click_pos.y;
			p.move_from(state->rmb.initial_p0, dx, dy);
			state->snap_active_translation();
		}
	}
	gtk_widget_queue_draw(widget);
}

static gboolean on_scroll(GtkEventControllerScroll*, double /*dx*/, double dy, gpointer user_data) {
	auto* state = static_cast<AppState*>(user_data);
	state->cycle_active((dy > 0) ? 1 : -1);
	state->redraw();
	return GDK_EVENT_STOP;
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
				Point target_pos = state->get_snapped_pos(
					Point{static_cast<int>(x), static_cast<int>(y)}
				);
				state->draft_measurement = std::make_unique<Measurement>(
					target_pos,
					target_pos,
					MeasurementOptions{ .show_diagonal = state->config.show_diagonal },
					state->config.guides
				);
				state->active_measurement = state->draft_measurement.get();
			}
			auto& m = *state->active_measurement;
			state->lmb.initial_p1 = m.start;
			state->lmb.initial_p2 = m.end;
			gtk_widget_queue_draw(widget);
			return GDK_EVENT_STOP;
		}
		else if (button == GDK_BUTTON_SECONDARY) {
			state->rmb.is_dragging = true;
			state->rmb.click_pos = Point{static_cast<int>(x), static_cast<int>(y)};

			if (
				state->active_measurement && (
					state->mode == Mode::Measurements ||
					state->active_measurement == state->draft_measurement.get()
				)
			) {
				auto& m = *state->active_measurement;
				state->rmb.initial_p1 = m.start;
				state->rmb.initial_p2 = m.end;
			}
			if (state->active_guide && state->mode == Mode::Guides) {
				auto& g = *state->active_guide;
				state->rmb.initial_p0 =
					g.orientation == Orientation::Horizontal ? Point{static_cast<int>(x), g.coord} :
					g.orientation == Orientation::Vertical ? Point{g.coord, static_cast<int>(y)} :
					Point{static_cast<int>(x), static_cast<int>(y)}
				;
			}
			if (state->active_color_pick && state->mode == Mode::ColorPicks) {
				auto& p = *state->active_color_pick;
				state->rmb.initial_p0 = p.pos;
				p.set_color(std::nullopt);
			}

			gtk_widget_queue_draw(widget);
			return GDK_EVENT_STOP;
		}
	}
	else if (type == GDK_BUTTON_RELEASE) {
		guint button = gdk_button_event_get_button(event);

		if (button == GDK_BUTTON_PRIMARY) {
			state->lmb.is_dragging = false;

			check_and_process(state);
			gtk_widget_queue_draw(widget);
			return GDK_EVENT_STOP;
		}
		else if (button == GDK_BUTTON_SECONDARY) {
			state->rmb.is_dragging = false;

			if (state->lmb.is_dragging && state->active_measurement) {
				auto& m = *state->active_measurement;
				state->lmb.click_pos = state->cursor.pos;
				state->lmb.initial_p1 = m.start;
				state->lmb.initial_p2 = m.end;
			}
			if (state->active_color_pick && state->mode == Mode::ColorPicks) {
				auto& p = *state->active_color_pick;
				p.set_color(state->get_pixel_color(p.pos.x, p.pos.y));
			}

			check_and_process(state);
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

	auto it = state->config.color_schemes.find("highlight");
	const ColorScheme* highlight_theme =
		(it != state->config.color_schemes.end())
			? &it->second
			: state->color_scheme
	;
	if (state->show_color_picks) {
		for (const auto& p : state->color_picks) {
			draw_color_pick(
				cr,
				*p,
				state->active_color_pick == p.get()
					? *highlight_theme
					: *state->color_scheme
				,
				state->config.text_style
			);
		}
	}
	if (state->show_guides) {
		for (const auto& g : state->guides) {
			draw_guide(
				cr,
				*g,
				width,
				height,
				state->active_guide == g.get()
					? *highlight_theme
					: *state->color_scheme
			);
		}
	}
	for (const auto& m : state->measurements) {
		draw_measurement(
			cr,
			*m,
			state->active_measurement == m.get()
				? *highlight_theme
				: *state->color_scheme
			,
			state->config.text_style
		);
	}
	if (state->draft_measurement) {
		draw_measurement(
			cr, *state->draft_measurement,
			*state->color_scheme,
			state->config.text_style
		);
	}
	draw_cursor(cr, state->cursor, width, height, state->color_scheme->basic);
}

class ScopedOverlayHider {
public:
	explicit ScopedOverlayHider(GtkWindow* window, AppState* state) : window_(window), state_(state) {
		GtkWidget* child = gtk_window_get_child(window_);
		const bool is_drawing_area = child && GTK_IS_DRAWING_AREA(child);
		if (is_drawing_area) {
			drawing_area_ = GTK_DRAWING_AREA(child);
			gtk_drawing_area_set_draw_func(
				drawing_area_,
				[](GtkDrawingArea*, cairo_t* cr, int, int, gpointer) {
					cairo_save(cr);
					cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
					cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
					cairo_paint(cr);
					cairo_restore(cr);
				},
				nullptr, nullptr
			);
			gtk_widget_queue_draw(GTK_WIDGET(drawing_area_));
		}
		while (g_main_context_pending(nullptr)) {
			g_main_context_iteration(nullptr, FALSE);
		}
		GdkDisplay* display = gdk_display_get_default();
		if (display) gdk_display_flush(display);
		g_usleep(20'000);
	}
	~ScopedOverlayHider() {
		if (drawing_area_) {
			gtk_drawing_area_set_draw_func(drawing_area_, on_draw, state_, nullptr);
			gtk_widget_queue_draw(GTK_WIDGET(drawing_area_));
		}
	}
private:
	GtkWindow* window_;
	AppState* state_;
	GtkDrawingArea* drawing_area_{nullptr};
};

static gboolean on_key_pressed(
	GtkEventControllerKey*,
	guint,
	guint keycode,
	GdkModifierType,
	gpointer user_data
) {
	return ShortcutManager::handle_key(keycode, true, *static_cast<AppState*>(user_data));
}
static gboolean on_key_released(
	GtkEventControllerKey*,
	guint,
	guint keycode,
	GdkModifierType,
	gpointer user_data
) {
	return ShortcutManager::handle_key(keycode, false, *static_cast<AppState*>(user_data));
}

void setup_main_window(
	GtkApplication* app,
	AppState& state,
	const Config& config
) {
	GtkWidget* window = gtk_application_window_new(app);

	if (!gtk_layer_is_supported()) {
		g_printerr("CRITICAL: gtk_layer_is_supported() returned FALSE!\n");
		GdkDisplay* display = gdk_display_get_default();
		if (display) {
			g_printerr("Current GDK Display type: %s\n", G_OBJECT_TYPE_NAME(display));
		}
	} else {
		g_print("INFO: Layer shell IS supported on this display!\n");
	}

	GtkCssProvider* provider = gtk_css_provider_new();
	gtk_css_provider_load_from_string(
		provider,
		"window, window.background, contents, drawingarea {\n"
		"  background-color: transparent;\n"
		"  background-image: none;\n"
		"  box-shadow: none;\n"
		"}"
	);
	gtk_style_context_add_provider_for_display(
		gdk_display_get_default(),
		GTK_STYLE_PROVIDER(provider),
		GTK_STYLE_PROVIDER_PRIORITY_USER + 100
	);
	g_object_unref(provider);

	gtk_layer_init_for_window(GTK_WINDOW(window));
	gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_OVERLAY);
	gtk_layer_set_keyboard_mode(GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

	gtk_layer_set_exclusive_zone(GTK_WINDOW(window), -1);

	GdkDisplay* display = gdk_display_get_default();
	GtkWidget* drawing_area = gtk_drawing_area_new();
	GdkClipboard* clipboard = gdk_display_get_clipboard(display);

	state.request_redraw = [drawing_area]() {
		gtk_widget_queue_draw(drawing_area);
	};
	state.request_text_to_clipboard = [clipboard](const std::string& text) {
		gdk_clipboard_set_text(clipboard, text.c_str());
	};
	state.request_screenshot = [window, &state](int x, int y, int w, int h) {
		ScopedOverlayHider hider(GTK_WINDOW(window), &state);
		if (auto texture = platform::get_region_texture(GTK_WINDOW(window), x, y, w, h)) {
			const auto target = state.config.screenshot.target;
			if (target != ScreenshotTarget::Clipboard) {
				texture_to_file(texture, state.config.screenshot.file_pattern);
			}
			if (target != ScreenshotTarget::File) {
				texture_to_clipboard(texture);
			}
			g_object_unref(texture);
		}
	};
	state.request_get_pixel_color = [window, &state](int x, int y) {
		ScopedOverlayHider hider(GTK_WINDOW(window), &state);
		return platform::get_pixel_color(GTK_WINDOW(window), x, y);
	};
	state.request_color_to_clipboard = [window, &state](Color& color) {
		GdkDisplay* display = gdk_display_get_default();
		GdkClipboard* clipboard = gdk_display_get_clipboard(display);
		gdk_clipboard_set_text(clipboard, utils::color_to_fmt_str(color).c_str());
	};

	gtk_window_set_child(GTK_WINDOW(window), drawing_area);
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), on_draw, &state, nullptr);

	GdkCursor* invisible_cursor = create_invisible_cursor(display);
	gtk_widget_set_cursor(drawing_area, invisible_cursor);
	g_object_unref(invisible_cursor);

	GtkEventController* motion_controller = gtk_event_controller_motion_new();
	g_signal_connect(motion_controller, "motion", G_CALLBACK(on_motion), &state);
	g_signal_connect(motion_controller, "enter", G_CALLBACK(on_motion), &state);
	gtk_widget_add_controller(drawing_area, motion_controller);

	GtkEventController* scroll_controller = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
	g_signal_connect(scroll_controller, "scroll", G_CALLBACK(on_scroll), &state);
	gtk_widget_add_controller(drawing_area, scroll_controller);

	GtkEventController* legacy_controller = gtk_event_controller_legacy_new();
	g_signal_connect(legacy_controller, "event", G_CALLBACK(on_legacy_event), &state);
	gtk_widget_add_controller(drawing_area, legacy_controller);

	GtkEventController* key_controller = gtk_event_controller_key_new();
	g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), &state);
	g_signal_connect(key_controller, "key-released", G_CALLBACK(on_key_released), &state);
	gtk_widget_add_controller(window, key_controller);

	gtk_window_present(GTK_WINDOW(window));
}
