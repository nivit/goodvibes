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

#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include <glib-object.h>

#include "framework/log.h"
#include "framework/gv-framework.h"

GList *gv_framework_object_list;

void
gv_framework_register(gpointer data)
{
	GObject *object = G_OBJECT(data);

	/* Add to the object list (we don't take ownership) */
	gv_framework_object_list = g_list_prepend(gv_framework_object_list, object);

	/* Add a weak pointer */
	g_object_add_weak_pointer(object, (gpointer *) &(gv_framework_object_list->data));
}

void
gv_framework_cleanup(void)
{
	GList *item;

	/* Objects in list should be empty, thanks to the magic of weak pointers */
	for (item = gv_framework_object_list; item; item = item->next) {
		GObject *object = G_OBJECT(item->data);

		if (object != NULL)
			WARNING("Object of type '%s' has not been finalized !",
			        G_OBJECT_TYPE_NAME(object));
	}

	/* Free list */
	g_list_free(gv_framework_object_list);
}

void
gv_framework_init(void)
{
	/* Dummy */
}
