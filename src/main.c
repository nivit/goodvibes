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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>

#include <glib.h>
#include <gio/gio.h>
#include <glib-unix.h>

#include "additions/glib.h"

#include "framework/log.h"

#include "framework/gv-framework.h"
#include "core/gv-core.h"
#ifdef UI_ENABLED
#include <gtk/gtk.h>
#include "ui/gv-ui.h"
#endif

#include "options.h"

/*
 * Print informations about program.
 */

static const gchar *
string_package_info(void)
{
	return PACKAGE_CAMEL_NAME " " PACKAGE_VERSION;
}

static const gchar *
string_copyright(void)
{
	return PACKAGE_COPYRIGHT " " PACKAGE_AUTHOR_NAME " " PACKAGE_AUTHOR_EMAIL;
}

static const gchar *
string_date_now(void)
{
	GDateTime *now;
	static gchar *text;

	g_free(text);

	now = g_date_time_new_now_local();
	text = g_date_time_format(now, "%F, %T");

	g_date_time_unref(now);

	return text;
}

static const gchar *
string_compile_libraries(void)
{
	static gchar *text;

	if (text == NULL)
		text = g_strjoin(", ",
		                 glib_get_compile_version_string(),
		                 gv_core_audio_backend_compile_version_string(),
#ifdef UI_ENABLED
		                 gv_ui_toolkit_compile_version_string(),
#endif
		                 NULL);

	return text;
}

static const gchar *
string_runtime_libraries(void)
{
	static gchar *text;

	if (text == NULL)
		text = g_strjoin(", ",
		                 glib_get_runtime_version_string(),
		                 gv_core_audio_backend_runtime_version_string(),
#ifdef UI_ENABLED
		                 gv_ui_toolkit_runtime_version_string(),
#endif
		                 NULL);

	return text;
}

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
 * Main - this is where everything starts...
 */

static void
on_shutdown(GApplication *application G_GNUC_UNUSED,
            gpointer      user_data G_GNUC_UNUSED)
{
#ifdef UI_ENABLED
	if (!options.without_ui) {
		DEBUG("---- Cooling down ui ----");
		gv_ui_cool_down();
	}
#endif

	DEBUG("---- Cooling down core ----");
	gv_core_cool_down();

#ifdef UI_ENABLED
	if (!options.without_ui) {
		DEBUG("---- Cleaning up ui ----");
		gv_ui_cleanup();
	}
#endif

	DEBUG("---- Cleaning up core ----");
	gv_core_cleanup();

	DEBUG("---- Cleaning up framework ----");
	gv_framework_cleanup();
}

static void
on_startup(GApplication *application G_GNUC_UNUSED,
           gpointer      user_data G_GNUC_UNUSED)
{
	DEBUG("---- Initializing framework ----");
	gv_framework_init();

	DEBUG("---- Initializing core ----");
	gv_core_init();

#ifdef UI_ENABLED
	if (!options.without_ui) {
		DEBUG("---- Initializing ui ----");
		gv_ui_init();
	}
#endif

	DEBUG("---- Peeping into lists ----");
	DEBUG("%s", stringify_list("Feature list     : ", gv_framework_feature_list));
	DEBUG("%s", stringify_list("Configurable list: ", gv_framework_configurable_list));
	DEBUG("%s", stringify_list("Errorable list   : ", gv_framework_errorable_list));

	DEBUG("---- Warming up core ----");
	gv_core_warm_up(options.uri_to_play);

#ifdef UI_ENABLED
	if (!options.without_ui) {
		DEBUG("---- Warming up ui ----");
		gv_ui_warm_up();
	}
#endif

	g_application_hold(application);
}

void
on_activate(GApplication *application G_GNUC_UNUSED,
            gpointer      user_data G_GNUC_UNUSED)
{
	INFO("Activated !");
}

static gboolean
sigint_handler(gpointer user_data)
{
	GApplication *application = G_APPLICATION(user_data);

	/* There's probably a '^C' written on the console line by now.
	 * Let's start a new line to keep logs clean.
	 */
	putchar('\n');

	/* Stop application */
	g_application_quit(application);

	return FALSE;
}

int
main(int argc, char *argv[])
{
	GApplication *app;
	int ret;

	/* Initialize i18n */
	setlocale(LC_ALL, NULL);
	bindtextdomain(PACKAGE_NAME, LOCALE_DIR);
	bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
	textdomain(PACKAGE_NAME);

	/* Set application name */
	g_set_prgname(PACKAGE_NAME);
	g_set_application_name(PACKAGE_LONG_NAME);

	/* Parse command-line, and run some init code at the same time */
	options_parse(&argc, &argv);

	/* We might just want to print the version and exit */
	if (options.print_version) {
		g_print("%s\n", string_package_info());
		g_print("%s\n", string_copyright());
		g_print("Compiled against: %s\n", string_compile_libraries());
		g_print("Running along   : %s\n", string_runtime_libraries());
		return EXIT_SUCCESS;
	}

	/* Run in background.
	 * This option may be used along with the 'logfile' option, therefore the
	 * arguments for daemon() must be as follow:
	 * - nochdir = 1: we MUST NOT change the working directory
	 *   (otherwise a relative path for logfile won't work)
	 * - noclose = 0: we MUST close std{in/out/err} BEFORE initializing the logs
	 *   (and let the log system re-open std{out/err} afterward if needed).
	 */
	if (options.background) {
		if (daemon(1, 0) == -1) {
			g_printerr("Failed to daemonize: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
	}

	/* Initialize log system, warm it up with a few logs */
	log_init(options.log_level, options.colorless, options.output_file);
	INFO("%s", string_package_info());
	INFO("%s", string_copyright());
	INFO("Started on %s, with pid %ld", string_date_now(), (long) getpid());
	INFO("Compiled against: %s", string_compile_libraries());
	INFO("Running along   : %s", string_runtime_libraries());

	/* Create the application */
#ifdef UI_ENABLED
	// TODO: avoid direct gtk call here
	app = G_APPLICATION(gtk_application_new("org." PACKAGE_CAMEL_NAME, G_APPLICATION_FLAGS_NONE));
#else
	app = g_application_new("org." PACKAGE_CAMEL_NAME, G_APPLICATION_FLAGS_NONE);
#endif

	g_signal_connect(app, "startup",  G_CALLBACK(on_startup),  NULL);
	g_signal_connect(app, "shutdown", G_CALLBACK(on_shutdown), NULL);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate),  NULL);

	/* Hold now, release on SIGINT */
	g_unix_signal_add(SIGINT, sigint_handler, app);

	/* Run */
	DEBUG(">>>> Running the application <<<<");
	ret = g_application_run(app, argc, argv);
	DEBUG(">>>> Application terminated <<<<");

	/* Cleanup */
	log_cleanup();
	options_cleanup();

	return ret;
}
