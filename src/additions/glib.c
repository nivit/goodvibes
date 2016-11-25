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

#include <string.h>
#include <glib.h>

/*
 * Version Information
 */

const gchar *
glib_get_runtime_version_string(void)
{
	static gchar *version_string;

	if (version_string == NULL) {
		version_string = g_strdup_printf("GLib %u.%u.%u",
		                                 glib_major_version,
		                                 glib_minor_version,
		                                 glib_micro_version);
	}

	return version_string;
}

const gchar *
glib_get_compile_version_string(void)
{
	static gchar *version_string;

	if (version_string == NULL) {
		version_string = g_strdup_printf("GLib %u.%u.%u",
		                                 GLIB_MAJOR_VERSION,
		                                 GLIB_MINOR_VERSION,
		                                 GLIB_MICRO_VERSION);
	}

	return version_string;
}

/*
 * String Utilities Functions
 */

gchar *
g_strjoin_null(const gchar *separator, unsigned int n_strings, ...)
{
	gchar *string, *s;
	va_list args;
	gsize len;
	gsize separator_len;
	gchar *ptr;
	guint i;

	if (separator == NULL)
		separator = "";

	separator_len = strlen(separator);

	/* First part, getting length */
	len = 1;
	va_start(args, n_strings);
	for (i = 0; i < n_strings; i++) {
		s = va_arg(args, gchar *);
		if (s) {
			if (len != 1)
				len += separator_len;
			len += strlen(s);
		}
	}
	va_end(args);

	/* Second part, building string */
	string = g_new(gchar, len);
	ptr = string;
	va_start(args, n_strings);
	for (i = 0; i < n_strings; i++) {
		s = va_arg(args, gchar *);
		if (s) {
			if (ptr != string)
				ptr = g_stpcpy(ptr, separator);
			ptr = g_stpcpy(ptr, s);
		}
	}
	va_end(args);

	if (ptr == string)
		string[0] = '\0';

	return string;
}

/*
 * GVariant
 */

void
g_variant_builder_add_dictentry_array_string(GVariantBuilder *b, const gchar *key, ...)
{
	GVariantBuilder ab;
	va_list ap;
	gchar *s;

	// TODO Check if string is consumed or duplicated.
	//      If duplicated, we probably need to free stuff in ock-dbus-server-*
	//      or otherwise, we just don't use g_object_get... ?

	g_variant_builder_init(&ab, G_VARIANT_TYPE_ARRAY);

	va_start(ap, key);
	s = va_arg(ap, gchar *);
	while (s) {
		GVariant *tmp = g_variant_new_string(s);
		g_variant_builder_add(&ab, "v", tmp);
		s = va_arg(ap, gchar *);
	}

	g_variant_builder_add(b, "{sv}", key, g_variant_builder_end(&ab));
}
