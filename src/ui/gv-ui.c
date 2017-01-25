/*
 * Goodvibes Radio Player
 *
 * Copyright (C) 2015-2017 Arnaud Rebillout
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

#include "additions/gtk.h"

#include "framework/gv-framework.h"

#include "core/gv-core.h"
#include "ui/gv-main-window.h"
#include "ui/gv-about-dialog.h"
#include "ui/gv-prefs-window.h"
#include "ui/gv-stock-icons.h"
#include "ui/gv-status-icon.h"

GSettings    *gv_ui_settings;

GvStatusIcon *gv_ui_status_icon;
GtkWidget    *gv_ui_main_window;
GtkWidget    *gv_ui_prefs_window;

/*
 * Underlying graphical toolkit
 */

GOptionGroup *
gv_ui_toolkit_init_get_option_group(void)
{
	/* Very not sure about the argument to pass here. From my experience:
	 * - if we don't use gtk_init(), we should pass TRUE.
	 * - if we use gtk_init(), passing FALSE is ok.
	 * Since we use GtkApplication which calls gtk_init() internally,
	 * let's pass FALSE and pray that it works.
	 */

	return gtk_get_option_group(FALSE);
}

const gchar *
gv_ui_toolkit_runtime_version_string(void)
{
	return gtk_get_runtime_version_string();
}

const gchar *
gv_ui_toolkit_compile_version_string(void)
{
	return gtk_get_compile_version_string();
}

/*
 * Ui public functions
 */

void
gv_ui_hide(void)
{
	/* In status icon mode, do nothing */
	if (gv_ui_status_icon)
		return;

	if (gv_ui_prefs_window)
		gtk_widget_destroy(gv_ui_prefs_window);

	gtk_widget_hide(gv_ui_main_window);
}

void
gv_ui_present_about(void)
{
	gv_show_about_dialog(GTK_WINDOW(gv_ui_main_window),
	                     gv_core_audio_backend_runtime_version_string(),
	                     gv_ui_toolkit_runtime_version_string());
}

void
gv_ui_present_preferences(void)
{
	if (gv_ui_prefs_window == NULL) {
		gv_ui_prefs_window = gv_prefs_window_new();
		g_object_add_weak_pointer(G_OBJECT(gv_ui_prefs_window),
		                          (gpointer *) &gv_ui_prefs_window);
	}

	gtk_window_present(GTK_WINDOW(gv_ui_prefs_window));
}

void
gv_ui_present_main(void)
{
	/* In status icon mode, do nothing */
	if (gv_ui_status_icon)
		return;

	gtk_window_present(GTK_WINDOW(gv_ui_main_window));
}

void
gv_ui_cleanup(void)
{
	/*
	 * Destroy ui objects
	 *
	 * Windows must be destroyed with gtk_widget_destroy().
	 * Forget about gtk_window_close() here, which seems to be asynchronous.
	 * Read the doc:
	 * https://developer.gnome.org/gtk3/stable/GtkWindow.html#gtk-window-new
	 */

	if (gv_ui_status_icon)
		g_object_unref(gv_ui_status_icon);

	if (gv_ui_prefs_window)
		gtk_widget_destroy(gv_ui_prefs_window);

	gtk_widget_destroy(gv_ui_main_window);

	/* Destroy settings */

	g_object_unref(gv_ui_settings);

	/* Stock icons */

	gv_stock_icons_cleanup();
}

void
gv_ui_init(GApplication *app, gboolean status_icon_mode)
{
	/* Stock icons */

	gv_stock_icons_init();

	/* Create settings */

	gv_ui_settings = g_settings_new(PACKAGE_APPLICATION_ID ".Ui");
	gv_framework_register(gv_ui_settings);

	/* Create ui objects */

	gv_ui_main_window = gv_main_window_new(app);
	gv_framework_register(gv_ui_main_window);

	if (status_icon_mode) {
		/* Configure window for popup mode */
		gv_main_window_configure_for_popup(GV_MAIN_WINDOW(gv_ui_main_window));

		/* Create a status icon, and we're done */
		gv_ui_status_icon = gv_status_icon_new(GTK_WINDOW(gv_ui_main_window));
		gv_framework_register(gv_ui_status_icon);
	} else {
		/* Configure window for standalone mode */
		gv_main_window_configure_for_standalone(GV_MAIN_WINDOW(gv_ui_main_window));

		/* No status icon */
		gv_ui_status_icon = NULL;
	}

	gv_main_window_populate_stations(GV_MAIN_WINDOW(gv_ui_main_window));
}
