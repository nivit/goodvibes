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

#include <gtk/gtk.h>

#include "framework/ock-file-helpers.h"

void
ock_builder_load(const char *filename, GtkBuilder **builder_out, gchar **uifile_out)
{
	GtkBuilder *builder;
	gchar *uifile;

	g_return_if_fail(builder_out != NULL);
	g_return_if_fail(uifile_out != NULL);

	/* Find the location of the ui file */
	uifile = ock_get_first_existing_path(OCK_DIR_CURRENT_DATA | OCK_DIR_SYSTEM_DATA,
	                                     filename);
	g_assert(uifile);

	/* Build ui from file */
	builder = gtk_builder_new_from_file(uifile);

	/* Fill output parameters */
	*builder_out = builder;
	*uifile_out = uifile;
}

