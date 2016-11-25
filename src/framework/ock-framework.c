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

#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include <glib-object.h>
#include <glib-unix.h>

#include "libgszn/gszn.h"

#include "framework/log.h"
#include "framework/ock-framework.h"

GList *ock_framework_feature_list;
GList *ock_framework_configurable_list;
GList *ock_framework_errorable_list;

static GMainLoop *main_loop;

static void
value_transform_bool_string_lowercase(const GValue *src_value, GValue *dest_value)
{
	dest_value->data[1].v_uint = G_VALUE_NOCOPY_CONTENTS;
	dest_value->data[0].v_pointer = src_value->data[0].v_int ? "true" : "false";
}

static gboolean
sigint_handler(gpointer user_data G_GNUC_UNUSED)
{
	/* There's probably a '^C' written on the console line by now.
	 * Let's start a new line to keep logs clean.
	 */
	putchar('\n');

	/* Stop program execution */
	ock_framework_quit_loop();
	return FALSE;
}

void
ock_framework_quit_loop(void)
{
	/* Quit the main loop */
	if (g_main_loop_is_running(main_loop))
		g_main_loop_quit(main_loop);
}

void
ock_framework_run_loop(void)
{
	/* Be ready to catch interruptions */
	g_unix_signal_add(SIGINT, sigint_handler, NULL);

	/* Run the main loop */
	g_assert(g_main_loop_is_running(main_loop) == FALSE);
	g_main_loop_run(main_loop);
}

void
ock_framework_cleanup(void)
{
	/* Lists should be empty by now */
	if (ock_framework_feature_list)
		WARNING("Feature list not empty, memory is leaked !");
	if (ock_framework_configurable_list)
		WARNING("Configurable list not empty, memory is leaked !");
	if (ock_framework_errorable_list)
		WARNING("Errorable list not empty, memory is leaked !");

	/* Cleanup GObject Serialization */
	gszn_cleanup();

	/* Unref the main loop */
	g_main_loop_unref(main_loop);
	main_loop = NULL;
}

void
ock_framework_init(void)
{
	/* Register a custom function to transform boolean to string.
	 * This function is used during serialization process, and therefore
	 * controls the appearance of a boolean in the configuration file.
	 * And lowercase looks better, that's all.
	 */
	g_value_register_transform_func(G_TYPE_BOOLEAN, G_TYPE_STRING,
	                                value_transform_bool_string_lowercase);

	/* Create the main loop */
	main_loop = g_main_loop_new(NULL, FALSE);

	/* Init GObject Serialization */
	gszn_init();

	/* Init lists - already intialized to NULL */
}
