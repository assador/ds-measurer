#include "app_state.hpp"
#include <gtk/gtk.h>

void AppState::queue_draw() const {
	if (drawing_area) gtk_widget_queue_draw(drawing_area);
}
