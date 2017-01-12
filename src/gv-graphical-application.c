/*
 * Goodvibes Radio Player
 *
 * Copyright (C) 2017 Arnaud Rebillout
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

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "framework/gv-framework.h"
#include "core/gv-core.h"
#include "ui/gv-ui.h"
#include "ui/gv-ui-helpers.h"

#include "gv-graphical-application.h"
#include "options.h"

/*
 * GObject definitions
 */

struct _GvGraphicalApplication {
	/* Parent instance structure */
	GtkApplication parent_instance;
};

G_DEFINE_TYPE(GvGraphicalApplication, gv_graphical_application, GTK_TYPE_APPLICATION)

/*
 * Helpers
 */

static const gchar *
stringify_list(const gchar *prefix, GList *list)
{
	GList *item;
	GString *str;
	static gchar *text;

	str = g_string_new(prefix);
	g_string_append(str, "[");

	for (item = list; item; item = item->next) {
		GObject *object;
		const gchar *object_name;

		object = item->data;
		object_name = G_OBJECT_TYPE_NAME(object);

		g_string_append_printf(str, "%s, ", object_name);
	}

	if (list != NULL)
		g_string_set_size(str, str->len - 2);

	g_string_append(str, "]");

	g_free(text);
	text = g_string_free(str, FALSE);

	return text;
}

/*
 * Public methods
 */

GApplication *
gv_graphical_application_new(void)
{
	return G_APPLICATION(g_object_new(GV_TYPE_GRAPHICAL_APPLICATION,
	                                  "application-id", "org." PACKAGE_CAMEL_NAME,
	                                  "flags", G_APPLICATION_FLAGS_NONE,
	                                  NULL));
}

/*
 * GApplication actions
 */

static void
preferences_action_cb(GSimpleAction *action G_GNUC_UNUSED,
                      GVariant      *parameters G_GNUC_UNUSED,
                      gpointer       user_data G_GNUC_UNUSED)
{
	gv_ui_present_preferences();
}

static void
help_action_cb(GSimpleAction *action G_GNUC_UNUSED,
               GVariant      *parameters G_GNUC_UNUSED,
               gpointer       user_data G_GNUC_UNUSED)
{
	g_app_info_launch_default_for_uri(PACKAGE_WEBSITE, NULL, NULL);
}

static void
about_action_cb(GSimpleAction *action G_GNUC_UNUSED,
                GVariant      *parameters G_GNUC_UNUSED,
                gpointer       user_data G_GNUC_UNUSED)
{
	gv_ui_present_about();
}

static void
close_ui_action_cb(GSimpleAction *action G_GNUC_UNUSED,
                   GVariant      *parameters G_GNUC_UNUSED,
                   gpointer       user_data G_GNUC_UNUSED)
{
	gv_ui_hide();
}

static void
quit_action_cb(GSimpleAction *action G_GNUC_UNUSED,
               GVariant      *parameters G_GNUC_UNUSED,
               gpointer       user_data G_GNUC_UNUSED)
{
	gv_core_quit();
}

static const GActionEntry gv_graphical_application_actions[] = {
	{ "preferences", preferences_action_cb, NULL, NULL, NULL, {0} },
	{ "help",        help_action_cb,        NULL, NULL, NULL, {0} },
	{ "about",       about_action_cb,       NULL, NULL, NULL, {0} },
	{ "close-ui",    close_ui_action_cb,    NULL, NULL, NULL, {0} },
	{ "quit",        quit_action_cb,        NULL, NULL, NULL, {0} },
	{ NULL,          NULL,                  NULL, NULL, NULL, {0} }
};

/*
 * GApplication methods
 */

static void
gv_graphical_application_shutdown(GApplication *app)
{
	DEBUG(">>>> Shutting down application <<<<");

	/* Cool down */
	DEBUG("---- Cooling down ui ----");
	gv_ui_cool_down();

	DEBUG("---- Cooling down core ----");
	gv_core_cool_down();

	/* Cleanup */
	DEBUG("---- Cleaning up ui ----");
	gv_ui_cleanup();

	DEBUG("---- Cleaning up core ----");
	gv_core_cleanup();

	DEBUG("---- Cleaning up framework ----");
	gv_framework_cleanup();

	/* Mandatory chain-up */
	G_APPLICATION_CLASS(gv_graphical_application_parent_class)->shutdown(app);
}

static void
gv_graphical_application_startup(GApplication *app)
{
	gboolean prefers_app_menu;

	DEBUG(">>>> Starting application <<<<");

	/* Mandatory chain-up, see:
	 * https://developer.gnome.org/gtk3/stable/GtkApplication.html#gtk-application-new
	 */
	G_APPLICATION_CLASS(gv_graphical_application_parent_class)->startup(app);

	/* Add actions to the application */
	g_action_map_add_action_entries(G_ACTION_MAP(app),
	                                gv_graphical_application_actions,
	                                -1,
	                                NULL);

	/* The 'close-ui' action makes no sense in the status icon mode */
	if (options.status_icon)
		g_action_map_remove_action(G_ACTION_MAP(app), "close-ui");

	/* Check how the application prefers to display it's main menu */
	prefers_app_menu = gtk_application_prefers_app_menu(GTK_APPLICATION(app));
	DEBUG("Application prefers... %s", prefers_app_menu ? "app-menu" : "menubar");

	/* Gnome-based desktop environments prefer an application menu.
	 * Legacy mode also needs this app menu, although it won't be displayed,
	 * but instead it will be brought up with a right-click.
	 */
	if (prefers_app_menu || options.status_icon == TRUE) {
		GtkBuilder *builder;
		GMenuModel *model;
		gchar *uifile;

		gv_builder_load("ui/app-menu.glade", &builder, &uifile);
		model = G_MENU_MODEL(gtk_builder_get_object(builder, "app-menu"));
		gtk_application_set_app_menu(GTK_APPLICATION(app), model);
		DEBUG("App menu set from ui file '%s'", uifile);
		g_free(uifile);
		g_object_unref(builder);
	}

	/* Unity-based and traditional desktop environments prefer a menu bar */
	if (!prefers_app_menu && options.status_icon == FALSE) {
		GtkBuilder *builder;
		GMenuModel *model;
		gchar *uifile;

		gv_builder_load("ui/menubar.glade", &builder, &uifile);
		model = G_MENU_MODEL(gtk_builder_get_object(builder, "menubar"));
		gtk_application_set_menubar(GTK_APPLICATION(app), model);
		DEBUG("Menubar set from ui file '%s'", uifile);
		g_free(uifile);
		g_object_unref(builder);
	}

	/* Initialization */
	DEBUG("---- Initializing framework ----");
	gv_framework_init();

	DEBUG("---- Initializing core ----");
	gv_core_init(app);

	DEBUG("---- Initializing ui ----");
	gv_ui_init(app, options.status_icon);

	/* Debug messages */
	DEBUG("---- Peeping into lists ----");
	DEBUG("%s", stringify_list("Feature list     : ", gv_framework_feature_list));
	DEBUG("%s", stringify_list("Configurable list: ", gv_framework_configurable_list));
	DEBUG("%s", stringify_list("Errorable list   : ", gv_framework_errorable_list));

	/* Warm-up */
	DEBUG("---- Warming up core ----");
	gv_core_warm_up();

	DEBUG("---- Warming up ui ----");
	gv_ui_warm_up();

	/* Hold application */
	// TODO: move that somewhere else
	g_application_hold(app);
}

static gboolean
when_idle_go_player(gpointer user_data)
{
	const gchar *uri_to_play = user_data;

	gv_player_go(gv_core_player, uri_to_play);

	return G_SOURCE_REMOVE;
}

static void
gv_graphical_application_activate(GApplication *app G_GNUC_UNUSED)
{
	static gboolean first_invocation = TRUE;

	DEBUG("Activated !");

	/* Present the main window */
	if (!options.without_ui)
		gv_ui_present_main();

	/* First invocation, schedule a callback to play music.
	 * DO NOT start playing now ! It's too early !
	 * There's still some init code pending, and we want to ensure
	 * (as much as possible) that this code is run before we start
	 * the playback. Therefore we schedule with a low priority.
	 */
	if (first_invocation) {
		first_invocation = FALSE;

		g_idle_add_full(G_PRIORITY_LOW, when_idle_go_player,
		                (void *) options.uri_to_play, NULL);
	}
}

/*
 * GObject methods
 */

static void
gv_graphical_application_init(GvGraphicalApplication *self)
{
	TRACE("%p", self);
}

static void
gv_graphical_application_class_init(GvGraphicalApplicationClass *class)
{
	GApplicationClass *application_class = G_APPLICATION_CLASS(class);

	TRACE("%p", class);

	/* Override GApplication methods */
	application_class->startup  = gv_graphical_application_startup;
	application_class->shutdown = gv_graphical_application_shutdown;
	application_class->activate = gv_graphical_application_activate;
}
