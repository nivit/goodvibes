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

#ifndef __OVERCOOKED_ADDITIONS_GLIB_OBJECT_H__
#define __OVERCOOKED_ADDITIONS_GLIB_OBJECT_H__

#include <glib-object.h>

/*
 * GType
 */

/* Duplicate type name, eventually without the prefix */
gchar *g_type_dup_name(GType type);
gchar *g_type_dup_name_no_prefix(GType type);

/*
 * GObject
 */

/* Duplicate type name, eventually without the prefix */
#define g_object_dup_type_name(obj)       \
	g_type_dup_name(G_OBJECT_TYPE(obj))
#define g_object_dup_type_name_no_prefix(obj)   \
	g_type_dup_name_no_prefix(G_OBJECT_TYPE(obj))

/* Chain up for finalize() */
#define G_OBJECT_CHAINUP_FINALIZE(module_obj_name, obj)   \
	G_OBJECT_CLASS(module_obj_name##_parent_class)->finalize(obj)

/* Chain up for constructed() - beware, constructed() is not guaranteed to exist */
#define G_OBJECT_CHAINUP_CONSTRUCTED(module_obj_name, obj)        \
	do { \
		if (G_OBJECT_CLASS(module_obj_name##_parent_class)->constructed) \
			G_OBJECT_CLASS(module_obj_name##_parent_class)->constructed(obj); \
	} while (0)

/* get() for fundamental types */
gboolean  g_object_get_boolean(GObject *object, const gchar *property_name);
guint     g_object_get_uint   (GObject *object, const gchar *property_name);
gchar    *g_object_get_string (GObject *object, const gchar *property_name);

const gchar *g_object_get_property_desc      (GObject *object, const gchar *property_name);
gboolean     g_object_get_property_uint_bounds(GObject *object, const gchar *property_name,
                                               guint *minimum, guint *maximum);

/*
 * Signals
 */

struct _GSignalHandler {
	const gchar *name;
	GCallback    callback;
};

typedef struct _GSignalHandler GSignalHandler;

void g_signal_handlers_connect(gpointer instance, GSignalHandler *handlers, gpointer data);
void g_signal_handlers_block  (gpointer instance, GSignalHandler *handlers, gpointer data);
void g_signal_handlers_unblock(gpointer instance, GSignalHandler *handlers, gpointer data);

#endif /* __OVERCOOKED_ADDITIONS_GLIB_OBJECT_H__ */
