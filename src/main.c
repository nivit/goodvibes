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
#include <glib-unix.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gio/gio.h>

#include "additions/glib.h"

#include "framework/log.h"
#include "core/gv-core.h"
#ifdef UI_ENABLED
#include "ui/gv-ui.h"
#endif

#ifdef UI_ENABLED
#include "gv-graphical-application.h"
#else
#include "gv-console-application.h"
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

/*
 * Main - this is where everything starts...
 */

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

#ifdef UI_ENABLED
	/* Set application icon */
	gtk_window_set_default_icon_name(PACKAGE_NAME);
#endif

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
	app = gv_graphical_application_new();
#else
	app = gv_console_application_new();
#endif

	/* Quit on SIGINT */
	g_unix_signal_add(SIGINT, sigint_handler, app);

	/* Run the application */
	ret = g_application_run(app, 0, NULL);

	/* Cleanup */
	log_cleanup();
	options_cleanup();

	return ret;
}
