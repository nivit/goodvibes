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

const gchar *SUPPORTED_URI_SCHEMES[] = {
	"http", "https", NULL
};

const gchar *SUPPORTED_MIME_TYPES[] = {
	"audio/*", NULL
};

gboolean
is_uri_scheme_supported(const gchar *uri)
{
	gchar *uri_scheme;
	const gchar **schemes = SUPPORTED_URI_SCHEMES;
	const gchar *scheme;
	gboolean uri_ok;

	uri_scheme = g_uri_parse_scheme(uri);

	uri_ok = FALSE;
	while (schemes && (scheme = *schemes++)) {
		if (g_strcmp0(scheme, uri_scheme) == 0) {
			uri_ok = TRUE;
			break;
		}
	}

	g_free(uri_scheme);

	return uri_ok;
}
