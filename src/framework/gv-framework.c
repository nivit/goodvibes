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

#include <glib.h>
#include <glib-object.h>

#include "libgszn/gszn.h"

#include "framework/log.h"
#include "framework/gv-framework.h"

GList *gv_framework_feature_list;
GList *gv_framework_configurable_list;
GList *gv_framework_errorable_list;

static void
value_transform_bool_string_lowercase(const GValue *src_value, GValue *dest_value)
{
	dest_value->data[1].v_uint = G_VALUE_NOCOPY_CONTENTS;
	dest_value->data[0].v_pointer = src_value->data[0].v_int ? "true" : "false";
}

void
gv_framework_quit_loop(void)
{
	// TODO: remove me
}

void
gv_framework_cleanup(void)
{
	/* Lists should be empty by now */
	if (gv_framework_feature_list)
		WARNING("Feature list not empty, memory is leaked !");
	if (gv_framework_configurable_list)
		WARNING("Configurable list not empty, memory is leaked !");
	if (gv_framework_errorable_list)
		WARNING("Errorable list not empty, memory is leaked !");

	/* Cleanup GObject Serialization */
	gszn_cleanup();
}

void
gv_framework_init(void)
{
	/* Register a custom function to transform boolean to string:
	 * use lowercase instead of the default uppercase.
	 * This function is used during serialization process, and therefore
	 * controls the appearance of a boolean in the configuration file.
	 * And lowercase looks better, that's all.
	 */
	g_value_register_transform_func(G_TYPE_BOOLEAN, G_TYPE_STRING,
	                                value_transform_bool_string_lowercase);

	/* Init GObject Serialization */
	gszn_init();

	/* Init lists - already intialized to NULL */
}
