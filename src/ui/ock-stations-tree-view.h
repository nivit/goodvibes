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

#ifndef __OVERCOOKED_UI_OCK_STATIONS_TREE_VIEW_H__
#define __OVERCOOKED_UI_OCK_STATIONS_TREE_VIEW_H__

#include <glib-object.h>
#include <gtk/gtk.h>

/* GObject declarations */

#define OCK_TYPE_STATIONS_TREE_VIEW ock_stations_tree_view_get_type()

G_DECLARE_FINAL_TYPE(OckStationsTreeView, ock_stations_tree_view, \
                     OCK, STATIONS_TREE_VIEW, GtkTreeView)

/* Methods */

GtkWidget *ock_stations_tree_view_new(void);

#endif /* __OVERCOOKED_UI_OCK_STATIONS_TREE_VIEW_H__ */
