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
ock_stock_icons_cleanup(void)
{
	/* Dummy */
}

void
ock_stock_icons_init(void)
{
	GtkIconTheme *theme;
	gchar *icon_dir;

	theme = gtk_icon_theme_get_default();

	/* Add './data/icons' to search path, for developers. */
	icon_dir = g_build_filename(ock_get_current_data_dir(), "icons", NULL);
	gtk_icon_theme_append_search_path(theme, icon_dir);
	g_free(icon_dir);
}
