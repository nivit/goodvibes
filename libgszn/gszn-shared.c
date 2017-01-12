/*
 * Libgszn
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

#include <string.h>
#include <glib.h>
#include <glib-object.h>

#include "gszn-error.h"

/* Transform functions */

gboolean
gszn_transform_string_to_value(const gchar *str, GValue *value, GError **err)
{
	g_return_val_if_fail(value != NULL, FALSE);

	/* If there's an existing string to value transform function, let's use it */
	if (g_value_type_transformable(G_TYPE_STRING, G_VALUE_TYPE(value))) {
		GValue tmp_value = G_VALUE_INIT;

		g_value_init(&tmp_value, G_TYPE_STRING);
		g_value_set_string(&tmp_value, str);
		g_value_transform(&tmp_value, value);
		g_value_unset(&tmp_value);

		return TRUE;
	}

	/* Otherwise, do it manually */
	if (G_VALUE_HOLDS_CHAR(value) ||
	    G_VALUE_HOLDS_UCHAR(value)) {
		GValue tmp_value = G_VALUE_INIT;

		if (strlen(str) != 1) {
			g_set_error(err, GSZN_ERROR, GSZN_ERROR_INVALID_VALUE,
			            "Invalid string '%s' for (u)char type", str);
			return FALSE;
		}

		g_value_init(&tmp_value, G_TYPE_UCHAR);
		g_value_set_uchar(&tmp_value, str[0]);
		g_value_transform(&tmp_value, value);
		g_value_unset(&tmp_value);

	} else if (G_VALUE_HOLDS_BOOLEAN(value)) {
		if (!g_ascii_strcasecmp(str, "true"))
			g_value_set_boolean(value, TRUE);
		else if (!g_ascii_strcasecmp(str, "false"))
			g_value_set_boolean(value, FALSE);
		else {
			g_set_error(err, GSZN_ERROR, GSZN_ERROR_INVALID_VALUE,
			            "Invalid string '%s' for boolean type", str);
			return FALSE;
		}

	} else if (G_VALUE_HOLDS_INT(value)   ||
	           G_VALUE_HOLDS_LONG(value)  ||
	           G_VALUE_HOLDS_INT64(value) ||
	           G_VALUE_HOLDS_ENUM(value)  ||
	           G_VALUE_HOLDS_FLAGS(value)) {
		GValue tmp_value = G_VALUE_INIT;
		gint64 int64;
		gchar *endptr;

		int64 = g_ascii_strtoll(str, &endptr, 10);
		if (endptr == str || *endptr != '\0') {
			g_set_error(err, GSZN_ERROR, GSZN_ERROR_INVALID_VALUE,
			            "Invalid string '%s' for signed integer type", str);
			return FALSE;
		}

		g_value_init(&tmp_value, G_TYPE_INT64);
		g_value_set_int64(&tmp_value, int64);
		g_value_transform(&tmp_value, value);
		g_value_unset(&tmp_value);

	} else if (G_VALUE_HOLDS_UINT(value)  ||
	           G_VALUE_HOLDS_ULONG(value) ||
	           G_VALUE_HOLDS_UINT64(value)) {
		GValue tmp_value = G_VALUE_INIT;
		guint64 uint64;
		gchar *endptr;

		uint64 = g_ascii_strtoull(str, &endptr, 10);
		if (endptr == str || *endptr != '\0') {
			g_set_error(err, GSZN_ERROR, GSZN_ERROR_INVALID_VALUE,
			            "Invalid string '%s' for unsigned integer type", str);
			return FALSE;
		}

		g_value_init(&tmp_value, G_TYPE_UINT64);
		g_value_set_uint64(&tmp_value, uint64);
		g_value_transform(&tmp_value, value);
		g_value_unset(&tmp_value);

	} else if (G_VALUE_HOLDS_FLOAT(value) ||
	           G_VALUE_HOLDS_DOUBLE(value)) {
		gdouble dbl;
		gchar *endptr;
		GValue tmp_value = G_VALUE_INIT;

		dbl = g_ascii_strtod(str, &endptr);
		if (endptr == str || *endptr != '\0') {
			g_set_error(err, GSZN_ERROR, GSZN_ERROR_INVALID_VALUE,
			            "Invalid string '%s' for floating type", str);
			return FALSE;
		}

		g_value_init(&tmp_value, G_TYPE_DOUBLE);
		g_value_set_double(&tmp_value, dbl);
		g_value_transform(&tmp_value, value);
		g_value_unset(&tmp_value);

	} else if (G_VALUE_HOLDS_STRING(value)) {
		g_value_set_string(value, str);

	} else {
		g_set_error(err, GSZN_ERROR, GSZN_ERROR_INVALID_VALUE,
		            "Can't transform string to value type '%s'", G_VALUE_TYPE_NAME(value));
		return FALSE;
	}

	return TRUE;
}

gboolean
gszn_transform_value_to_string(const GValue *value, gchar **str, GError **err)
{
	g_return_val_if_fail(str != NULL, FALSE);
	*str = NULL;

	/* If there's an existing value to string transform function, let's use it,
	 * even though the GLib documentation advises against. See:
	 * https://developer.gnome.org/gobject/stable/
	 * gobject-Generic-values.html#g-value-transform
	 */
	if (g_value_type_transformable(G_VALUE_TYPE(value), G_TYPE_STRING)) {
		GValue tmp_value = G_VALUE_INIT;

		g_value_init(&tmp_value, G_TYPE_STRING);
		g_value_transform(value, &tmp_value);
		*str = g_value_dup_string(&tmp_value);
		g_value_unset(&tmp_value);

		return TRUE;
	}

	/* Otherwise, do it manually */
	if (G_VALUE_HOLDS_CHAR(value) ||
	    G_VALUE_HOLDS_UCHAR(value)) {
		GValue tmp_value = G_VALUE_INIT;
		gchar string[2];

		g_value_init(&tmp_value, G_TYPE_UCHAR);
		g_value_transform(value, &tmp_value);
		string[0] = g_value_get_uchar(&tmp_value);
		string[1] = '\0';
		g_value_unset(&tmp_value);
		*str = g_strdup(string);

	} else if (G_VALUE_HOLDS_BOOLEAN(value)) {
		gboolean value_bool;

		value_bool = g_value_get_boolean(value);
		*str = g_strdup(value_bool ? "true" : "false");

	} else if (G_VALUE_HOLDS_INT(value)   ||
	           G_VALUE_HOLDS_LONG(value)  ||
	           G_VALUE_HOLDS_INT64(value) ||
	           G_VALUE_HOLDS_ENUM(value)  ||
	           G_VALUE_HOLDS_FLAGS(value)) {
		GValue tmp_value = G_VALUE_INIT;
		gint64 value_int64;

		g_value_init(&tmp_value, G_TYPE_INT64);
		g_value_transform(value, &tmp_value);
		value_int64 = g_value_get_int64(&tmp_value);
		g_value_unset(&tmp_value);
		*str = g_strdup_printf("%lld", (long long int) value_int64);

	} else if (G_VALUE_HOLDS_UINT(value)  ||
	           G_VALUE_HOLDS_ULONG(value) ||
	           G_VALUE_HOLDS_UINT64(value)) {
		GValue tmp_value = G_VALUE_INIT;
		guint64 value_uint64;

		g_value_init(&tmp_value, G_TYPE_UINT64);
		g_value_transform(value, &tmp_value);
		value_uint64 = g_value_get_uint64(&tmp_value);
		g_value_unset(&tmp_value);
		*str = g_strdup_printf("%llu", (long long unsigned) value_uint64);

	} else if (G_VALUE_HOLDS_FLOAT(value) ||
	           G_VALUE_HOLDS_DOUBLE(value)) {
		GValue tmp_value = G_VALUE_INIT;
		gdouble value_double;

		g_value_init(&tmp_value, G_TYPE_DOUBLE);
		g_value_transform(value, &tmp_value);
		value_double = g_value_get_double(&tmp_value);
		g_value_unset(&tmp_value);
		*str = g_strdup_printf("%lf", value_double);

	} else if (G_VALUE_HOLDS_STRING(value)) {
		const gchar *value_string;

		value_string = g_value_get_string(value);
		*str = g_strdup(value_string);

	} else {
		g_set_error(err, GSZN_ERROR, GSZN_ERROR_INVALID_VALUE,
		            "Can't transform value type '%s' to string", G_VALUE_TYPE_NAME(value));
		return FALSE;
	}

	return TRUE;
}

/* GObject helpers */

gchar *
gszn_get_object_uid(GObject *object)
{
	GParamSpec *pspec;
	gchar *str;

	pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(object), "gszn-uid");
	if (pspec == NULL)
		return NULL;

	if (pspec->value_type != G_TYPE_STRING) {
		g_warning("Invalid type for uid property, should be string");
		return NULL;
	}

	g_object_get(object, "gszn-uid", &str, NULL);

	return str;
}

void
gszn_set_object_uid(GObject *object, const gchar *uid)
{
	g_object_set(object, "gszn-uid", uid, NULL);
}
