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
#include <string.h>

#include <glib.h>

#include "core/gv-core.h"
#ifdef UI_ENABLED
#include "ui/gv-ui.h"
#endif

#include "options.h"

struct options options;

static GOptionEntry entries[] = {
	{
		"background", 'b', 0, G_OPTION_ARG_NONE, &options.background,
		"Run in the background", NULL
	},
	{
		"colorless", 'c', 0, G_OPTION_ARG_NONE, &options.colorless,
		"Disable colors in log messages", NULL
	},
	{
		"log-level", 'l', 0, G_OPTION_ARG_STRING, &options.log_level,
		"Log level (default is 'message')", "level"
	},
	{
		"output-file", 'o', 0, G_OPTION_ARG_STRING, &options.output_file,
		"Redirect output to a file", "file"
	},
#ifdef UI_ENABLED
	{
		"without-ui", 0, 0, G_OPTION_ARG_NONE, &options.without_ui,
		"Disable graphical user interface", NULL
	},
#endif
	{
		"version", 'v', 0, G_OPTION_ARG_NONE, &options.print_version,
		"Print the version and exit", NULL
	},
	{ .long_name = NULL }
};

static gchar *description = "Log levels, from the most chatty to the most quiet:\n"
                            "trace/debug/info/warning/critical/error";

static void
print_help(GOptionContext *context)
{
	gchar *help;

	help = g_option_context_get_help(context, TRUE, NULL);
	g_print("%s", help);
	g_free(help);

	exit(EXIT_FAILURE);
}

void
options_parse(int *argc, char **argv[])
{
	GError *error = NULL;
	GOptionContext *context;

	/* Init options */
	memset(&options, 0, sizeof(struct options));

	/* Create context & entries */
	context = g_option_context_new("- A radio player providing good vibrations");
	g_option_context_add_main_entries(context, entries, NULL);

	/* Add option groups and perform some init code at the same time */
	g_option_context_add_group(context, gv_core_audio_backend_init_get_option_group());
#ifdef UI_ENABLED
	g_option_context_add_group(context, gv_ui_toolkit_init_get_option_group());
#endif

	/* Add description */
	g_option_context_set_description(context, description);

	/* Parse the command-line arguments */
	if (!g_option_context_parse(context, argc, argv, &error)) {
		g_print("Failed to parse options: %s\n", error->message);
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

	/* There should be at most one argument left: the URI to play */
	switch (*argc) {
	case 1:
		options.uri_to_play = NULL;
		break;
	case 2:
		options.uri_to_play = (*argv)[1];
		break;
	default:
		print_help(context);
		exit(EXIT_FAILURE);
		break;
	}

	/* Leave no trace behind */
	g_option_context_free(context);
}

void
options_cleanup(void)
{
	/* Run some cleanup code that matches the init code done in parse() */
	gv_core_audio_backend_cleanup();
}
