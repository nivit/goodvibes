/*
 * Goodvibes Radio Player
 *
 * Copyright (C) 2015-2016 Arnaud Rebillout
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

#include "ui/gv-main-window.h"
#include "ui/gv-about-dialog.h"
#include "ui/gv-prefs-window.h"
#include "ui/gv-stock-icons.h"
#include "ui/gv-tray.h"

#ifdef HOTKEYS_ENABLED
#include "ui/feat/gv-hotkeys.h"
#endif
#ifdef NOTIFICATIONS_ENABLED
#include "ui/feat/gv-notifications.h"
#endif

GvTray    *gv_ui_tray;
GtkWidget *gv_ui_main_window;
GtkWidget *gv_ui_prefs_window;

static GvFeature *features[8];

void
gv_ui_quit(void)
{
	GtkWindow *main_window = GTK_WINDOW(gv_ui_main_window);
	GtkApplication *app = gtk_window_get_application(main_window);

	// TODO: should we idle add ?

	g_application_quit(G_APPLICATION(app));
}

void
gv_ui_present_about(void)
{
	gv_show_about_dialog(GTK_WINDOW(gv_ui_main_window));
}

void
gv_ui_present_preferences(void)
{
	GtkWidget *window = gv_ui_prefs_window;

	if (window == NULL) {
		window = gv_prefs_window_new();
		g_object_add_weak_pointer(G_OBJECT(window), (gpointer *) &gv_ui_prefs_window);
	}

	gtk_window_present(GTK_WINDOW(window));
}

void
gv_ui_present_main(void)
{
	GtkWidget *window = gv_ui_main_window;

	/* In status icon mode, do nothing */
	if (gv_ui_tray)
		return;

	gtk_window_present(GTK_WINDOW(window));
}

void
gv_ui_cool_down(void)
{
	/* Dummy */
}

void
gv_ui_warm_up(void)
{
	GvMainWindow *main_window = GV_MAIN_WINDOW(gv_ui_main_window);

	gv_main_window_populate_stations(main_window);
}

void
gv_ui_cleanup(void)
{
	/* Features */

	GvFeature *feature;
	guint idx;

	for (idx = 0, feature = features[0]; feature != NULL; feature = features[++idx]) {
		gv_framework_features_remove(feature);
		gv_framework_configurables_remove(feature);
		gv_framework_errorables_remove(feature);
		g_object_unref(feature);
	}

	/* Ui objects */

	/* Windows must be destroyed with gtk_widget_destroy().
	 * Forget about gtk_window_close() here, which seems to be asynchronous.
	 * Read the doc:
	 * https://developer.gnome.org/gtk3/stable/GtkWindow.html#gtk-window-new
	 */

	GtkWidget *prefs_window = gv_ui_prefs_window;
	GtkWidget *main_window  = gv_ui_main_window;
	GvTray    *tray         = gv_ui_tray;

	if (tray) {
		gv_framework_configurables_remove(tray);
		g_object_unref(tray);
	}

	if (prefs_window)
		gtk_widget_destroy(main_window);

	gv_framework_configurables_remove(main_window);
	gtk_widget_destroy(main_window);

	/* Stock icons */

	gv_stock_icons_cleanup();
}

void
gv_ui_init(GApplication *app, gboolean status_icon_mode)
{
	/* Stock icons */
	gv_stock_icons_init();



	/* ----------------------------------------------- *
	 * Ui objects                                      *
	 * ----------------------------------------------- */

	GtkWidget *main_window;
	GvTray    *tray;

	main_window = gv_main_window_new(app);
	gv_framework_configurables_append(main_window);

	if (status_icon_mode) {
		/* Configure window for popup mode */
		gv_main_window_configure_for_popup(GV_MAIN_WINDOW(main_window));

		/* Create a tray icon, and we're done */
		tray = gv_tray_new(GTK_WINDOW(main_window));
		gv_framework_configurables_append(tray);
	} else {
		/* Configure window for standalone mode */
		gv_main_window_configure_for_standalone(GV_MAIN_WINDOW(main_window));

		/* Present the window immediately */
		gtk_window_present(GTK_WINDOW(main_window));

		/* No tray icon */
		tray = NULL;
	}

	/* ----------------------------------------------- *
	 * Features                                        *
	 * ----------------------------------------------- */

	GvFeature *feature;
	guint idx;

	/* Create every feature enabled at compile-time */
	idx = 0;

#ifdef HOTKEYS_ENABLED
	feature = gv_feature_new(GV_TYPE_HOTKEYS, FALSE);
	features[idx++] = feature;
	gv_framework_features_append(feature);
	gv_framework_configurables_append(feature);
	gv_framework_errorables_append(feature);
#endif
#ifdef NOTIFICATIONS_ENABLED
	feature = gv_feature_new(GV_TYPE_NOTIFICATIONS, TRUE);
	features[idx++] = feature;
	gv_framework_features_append(feature);
	gv_framework_configurables_append(feature);
#endif



	/* ----------------------------------------------- *
	 * Initialize global variables                     *
	 * ----------------------------------------------- */

	gv_ui_tray = tray;
	gv_ui_main_window = main_window;
}

/*
 * Underlying toolkit
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
