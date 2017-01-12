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

#include <gtk/gtk.h>

#include "framework/gv-file-helpers.h"

void
gv_builder_load(const char *filename, GtkBuilder **builder_out, gchar **uifile_out)
{
	gchar *ui_filename;
	gchar *file_found;
	GtkBuilder *builder;

	g_return_if_fail(builder_out != NULL);

	/* Prepend the 'ui' prefix */
	ui_filename = g_build_filename("ui/", filename, NULL);

	/* Find the location of the ui file */
	file_found = gv_get_first_existing_path(GV_DIR_CURRENT_DATA | GV_DIR_SYSTEM_DATA,
	                                        ui_filename);
	g_assert_nonnull(file_found);
	g_free(ui_filename);

	/* Build ui from file */
	builder = gtk_builder_new_from_file(file_found);

	/* Fill output parameters */
	*builder_out = builder;
	if (uifile_out)
		*uifile_out = file_found;
	else
		g_free(file_found);
}
