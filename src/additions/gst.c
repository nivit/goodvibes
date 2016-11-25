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

#include <gst/gst.h>

/*
 * Gst
 */

const gchar *
gst_get_runtime_version_string(void)
{
	static gchar *version_string;

	if (version_string == NULL) {
		guint major;
		guint minor;
		guint micro;
		guint nano;

		gst_version(&major, &minor, &micro, &nano);

		version_string = g_strdup_printf("GStreamer %u.%u.%u.%u",
		                                 major, minor, micro, nano);
	}

	return version_string;
}

const gchar *
gst_get_compile_version_string(void)
{
	static gchar *version_string;

	if (version_string == NULL) {
		version_string = g_strdup_printf("GStreamer %u.%u.%u.%u",
		                                 GST_VERSION_MAJOR,
		                                 GST_VERSION_MINOR,
		                                 GST_VERSION_MICRO,
		                                 GST_VERSION_NANO);
	}

	return version_string;
}
