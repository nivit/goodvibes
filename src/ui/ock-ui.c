/*
 * Overcooked Radio Player
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

#include "framework/ock-framework.h"

#include "ui/ock-main-window.h"
#include "ui/ock-stock-icons.h"
#include "ui/ock-tray.h"

#ifdef HOTKEYS_ENABLED
#include "ui/feat/ock-hotkeys.h"
#endif
#ifdef NOTIFICATIONS_ENABLED
#include "ui/feat/ock-notifications.h"
#endif

OckTray   *ock_ui_tray;
GtkWidget *ock_ui_main_window;

static OckFeature *features[8];

void
ock_ui_cool_down(void)
{
	/* Dummy */
}

void
ock_ui_warm_up(void)
{
	/* Dummy */
}

void
ock_ui_cleanup(void)
{
	/* Features */

	OckFeature *feature;
	guint idx;

	for (idx = 0, feature = features[0]; feature != NULL; feature = features[++idx]) {
		ock_framework_features_remove(feature);
		ock_framework_configurables_remove(feature);
		ock_framework_errorables_remove(feature);
		g_object_unref(feature);
	}

	/* Ui objects */

	GtkWidget       *main_window   = ock_ui_main_window;
	OckTray         *tray          = ock_ui_tray;

	ock_framework_configurables_remove(tray);
	g_object_unref(tray);

	ock_framework_configurables_remove(main_window);
	gtk_widget_destroy(main_window);

	/* Stock icons */

	ock_stock_icons_cleanup();
}

void
ock_ui_init(void)
{
	/* Gtk+ must have been initialized beforehand, let's check that */
	g_assert(gtk_is_initialized());

	/* Stock icons */
	ock_stock_icons_init();



	/* ----------------------------------------------- *
	 * Ui objects                                      *
	 * ----------------------------------------------- */

	GtkWidget       *main_window;
	OckTray         *tray;

	main_window = ock_main_window_new();
	ock_framework_configurables_append(main_window);

	tray = ock_tray_new();
	ock_framework_configurables_append(tray);



	/* ----------------------------------------------- *
	 * Features                                        *
	 * ----------------------------------------------- */

	OckFeature *feature;
	guint idx;

	/* Create every feature enabled at compile-time */
	idx = 0;

#ifdef HOTKEYS_ENABLED
	feature = ock_feature_new(OCK_TYPE_HOTKEYS, FALSE);
	features[idx++] = feature;
	ock_framework_features_append(feature);
	ock_framework_configurables_append(feature);
	ock_framework_errorables_append(feature);
#endif
#ifdef NOTIFICATIONS_ENABLED
	feature = ock_feature_new(OCK_TYPE_NOTIFICATIONS, FALSE);
	features[idx++] = feature;
	ock_framework_features_append(feature);
	ock_framework_configurables_append(feature);
#endif



	/* ----------------------------------------------- *
	 * Initialize global variables                     *
	 * ----------------------------------------------- */

	ock_ui_tray = tray;
	ock_ui_main_window = main_window;
}

/*
 * Underlying toolkit
 */

GOptionGroup *
ock_ui_toolkit_init_get_option_group(void)
{
	GOptionGroup *group;

	/* According to the doc, there's no need to call gtk_init()
	 * if we invoke gtk_get_option_group().
	 * By peeping into the code, it seems that the argument for
	 * gtk_get_option_group() should be TRUE if we want the same
	 * behavior as gtk_init().
	 */
	group = gtk_get_option_group(TRUE);

	/* Gtk doesn't offer a function to check if it's initialized.
	 * So we do that manually with our homemade functions.
	 */
	gtk_set_initialized();

	return group;
}

const gchar *
ock_ui_toolkit_runtime_version_string(void)
{
	return gtk_get_runtime_version_string();
}

const gchar *
ock_ui_toolkit_compile_version_string(void)
{
	return gtk_get_compile_version_string();
}
