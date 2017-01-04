/*
 * Goodvibes Radio Player
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

#ifndef __GOODVIBES_UI_GV_STATIONS_TREE_VIEW_H__
#define __GOODVIBES_UI_GV_STATIONS_TREE_VIEW_H__

#include <glib-object.h>
#include <gtk/gtk.h>

/* GObject declarations */

#define GV_TYPE_STATIONS_TREE_VIEW gv_stations_tree_view_get_type()

G_DECLARE_FINAL_TYPE(GvStationsTreeView, gv_stations_tree_view, \
                     GV, STATIONS_TREE_VIEW, GtkTreeView)

/* Methods */

GtkWidget *gv_stations_tree_view_new(void);

gboolean   gv_stations_tree_view_has_context_menu(GvStationsTreeView *self);
void       gv_stations_tree_view_populate        (GvStationsTreeView *self);

#endif /* __GOODVIBES_UI_GV_STATIONS_TREE_VIEW_H__ */
