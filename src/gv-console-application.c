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
#include <gio/gio.h>

#include "framework/log.h"
#include "framework/gv-framework.h"
#include "core/gv-core.h"

#include "gv-console-application.h"
#include "options.h"

/*
 * GObject definitions
 */

struct _GvConsoleApplication {
	/* Parent instance structure */
	GApplication parent_instance;
};

G_DEFINE_TYPE(GvConsoleApplication, gv_console_application, G_TYPE_APPLICATION)

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
gv_console_application_new(void)
{
	return G_APPLICATION(g_object_new(GV_TYPE_CONSOLE_APPLICATION,
	                                  "application-id", "org." PACKAGE_CAMEL_NAME,
	                                  "flags", G_APPLICATION_FLAGS_NONE,
	                                  NULL));
}

/*
 * GApplication methods
 */

static void
gv_console_application_shutdown(GApplication *app)
{
	DEBUG(">>>> Shutting down application <<<<");

	/* Cool down */
	DEBUG("---- Cooling down core ----");
	gv_core_cool_down();

	/* Cleanup */
	DEBUG("---- Cleaning up core ----");
	gv_core_cleanup();

	DEBUG("---- Cleaning up framework ----");
	gv_framework_cleanup();

	/* Mandatory chain-up */
	G_APPLICATION_CLASS(gv_console_application_parent_class)->shutdown(app);
}

static void
gv_console_application_startup(GApplication *app)
{
	DEBUG(">>>> Starting application <<<<");

	/* Mandatory chain-up, see:
	 * https://developer.gnome.org/gtk3/stable/GtkApplication.html#gtk-application-new
	 */
	G_APPLICATION_CLASS(gv_console_application_parent_class)->startup(app);

	/* Initialization */
	DEBUG("---- Initializing framework ----");
	gv_framework_init();

	DEBUG("---- Initializing core ----");
	gv_core_init(app);

	/* Debug messages */
	DEBUG("---- Peeping into lists ----");
	DEBUG("%s", stringify_list("Feature list     : ", gv_framework_feature_list));
	DEBUG("%s", stringify_list("Configurable list: ", gv_framework_configurable_list));
	DEBUG("%s", stringify_list("Errorable list   : ", gv_framework_errorable_list));

	/* Warm-up */
	DEBUG("---- Warming up core ----");
	gv_core_warm_up();

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
gv_console_application_activate(GApplication *app G_GNUC_UNUSED)
{
	static gboolean first_invocation = TRUE;

	DEBUG("Activated !");

	/* First invocation, schedule a callback to play music.
	 * DO NOT start playing now ! It's too early !
	 * There's still some init code pending, and we want to ensure
	 * (as much as possible) that this code is run before we start
	 * the playback. Therefore we schedule with a low priority.
	 */
	if (first_invocation) {
		g_idle_add_full(G_PRIORITY_LOW, when_idle_go_player,
		                (void *) options.uri_to_play, NULL);
	}
}

/*
 * GObject methods
 */

static void
gv_console_application_init(GvConsoleApplication *self)
{
	TRACE("%p", self);
}

static void
gv_console_application_class_init(GvConsoleApplicationClass *class)
{
	GApplicationClass *application_class = G_APPLICATION_CLASS(class);

	TRACE("%p", class);

	/* Override GApplication methods */
	application_class->startup  = gv_console_application_startup;
	application_class->shutdown = gv_console_application_shutdown;
	application_class->activate = gv_console_application_activate;
}
