/*
 * Libcaphe
 *
 * Copyright (C) 2016 Arnaud Rebillout
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

#include "caphe-main.h"

CapheMain *caphe;

CapheMain *
caphe_get_default(void)
{
	g_return_val_if_fail(caphe != NULL, NULL);

	return caphe;
}

void
caphe_cleanup(void)
{
	g_return_if_fail(caphe != NULL);

	g_object_unref(caphe);
	caphe = NULL;
}

void
caphe_init(const gchar *application_name)
{
	g_return_if_fail(caphe == NULL);

	caphe = g_object_new(CAPHE_TYPE_MAIN,
	                     "application-name", application_name,
	                     NULL);
}
