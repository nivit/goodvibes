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

#include <glib-object.h>

#include "additions/glib-object.h"

/*
 * GType
 */

gchar *
g_type_dup_name(GType type)
{
	return g_strdup(g_type_name(type));
}

gchar *
g_type_dup_name_no_prefix(GType type)
{
	const gchar *type_name;
	const gchar *ptr;

	type_name = g_type_name(type);

	ptr = type_name;
	if (*ptr != '\0')
		ptr++;

	while (*ptr != '\0') {
		if (g_ascii_isupper(*ptr))
			break;
		ptr++;
	}

	if (*ptr == '\0')
		return g_strdup(type_name);
	else
		return g_strdup(ptr);

}

/*
 * GObject
 */

gboolean
g_object_get_boolean(GObject *object, const gchar *property_name)
{
	gboolean value;

	g_object_get(object, property_name, &value, NULL);

	return value;
}

guint
g_object_get_uint(GObject *object, const gchar *property_name)
{
	guint value;

	g_object_get(object, property_name, &value, NULL);

	return value;
}

gchar *
g_object_get_string(GObject *object, const gchar *property_name)
{
	gchar *value;

	g_object_get(object, property_name, &value, NULL);

	return value;
}

const gchar *
g_object_get_property_desc(GObject *object, const gchar *property_name)
{
	GParamSpec *pspec;
	const gchar *desc;

	pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(object), property_name);
	g_assert(pspec);
	desc = g_param_spec_get_blurb(pspec);
	g_assert(desc);

	return desc;
}

gboolean
g_object_get_property_uint_bounds(GObject *object, const gchar *property_name,
                                  guint *minimum, guint *maximum)
{
	GParamSpec *pspec;
	GParamSpecUInt *pspec_uint;

	// TODO Log or report errors ?

	if (minimum)
		*minimum = 0;
	if (maximum)
		*maximum = 0;

	pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(object), property_name);
	if (pspec == NULL)
		return FALSE;

	if (G_IS_PARAM_SPEC_UINT(pspec) == FALSE)
		return FALSE;

	pspec_uint = G_PARAM_SPEC_UINT(pspec);
	if (minimum)
		*minimum = pspec_uint->minimum;
	if (maximum)
		*maximum = pspec_uint->maximum;

	return TRUE;
}

/*
 * Signals
 */

void
g_signal_handlers_connect(gpointer instance, GSignalHandler *handlers, gpointer data)
{
	GSignalHandler *handler;

	if (handlers == NULL)
		return;

	for (handler = handlers; handler->name; handler++)
		g_signal_connect(instance, handler->name, handler->callback, data);
}

void
g_signal_handlers_block(gpointer instance, GSignalHandler *handlers, gpointer data)
{
	GSignalHandler *handler;

	if (handlers == NULL)
		return;

	for (handler = handlers; handler->name; handler++)
		g_signal_handlers_block_by_func(instance, handler->callback, data);
}

void
g_signal_handlers_unblock(gpointer instance, GSignalHandler *handlers, gpointer data)
{
	GSignalHandler *handler;

	if (handlers == NULL)
		return;

	for (handler = handlers; handler->name; handler++)
		g_signal_handlers_unblock_by_func(instance, handler->callback, data);
}
