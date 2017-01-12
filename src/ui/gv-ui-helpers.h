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

#ifndef __GOODVIBES_UI_GV_UI_HELPERS_H__
#define __GOODVIBES_UI_GV_UI_HELPERS_H__

#include <gtk/gtk.h>

/*
 * Gtk builder helpers
 */

void gv_builder_load(const char *filename, GtkBuilder **builder, gchar **uifile);

/*
 * Visual layout, according to:
 * https://developer.gnome.org/hig/stable/visual-layout.html
 */

#define GV_UI_WINDOW_BORDER 18
#define GV_UI_GROUP_SPACING 18
#define GV_UI_ELEM_SPACING  6
#define GV_UI_LABEL_SPACING 12

#endif /* __GOODVIBES_UI_GV_UI_HELPERS_H__ */
