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

#include <glib.h>
#include <gtk/gtk.h>

/*
 * Main Loop and Events
 */

static gboolean gtk_initialized;

void
gtk_set_initialized(void)
{
	gtk_initialized = TRUE;
}

gboolean
gtk_is_initialized(void)
{
	return gtk_initialized;
}

/*
 * Version Information
 */

const gchar *
gtk_get_runtime_version_string(void)
{
	static gchar *version_string;

	if (version_string == NULL) {
		version_string = g_strdup_printf("GTK+ %u.%u.%u",
		                                 gtk_get_major_version(),
		                                 gtk_get_minor_version(),
		                                 gtk_get_micro_version());
	}

	return version_string;
}

const gchar *
gtk_get_compile_version_string(void)
{
	static gchar *version_string;

	if (version_string == NULL) {
		version_string = g_strdup_printf("GTK+ %u.%u.%u",
		                                 GTK_MAJOR_VERSION,
		                                 GTK_MINOR_VERSION,
		                                 GTK_MICRO_VERSION);
	}

	return version_string;
}
