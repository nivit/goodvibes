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

#ifndef __OVERCOOKED_UI_OCK_STATION_CONTEXT_MENU_H__
#define __OVERCOOKED_UI_OCK_STATION_CONTEXT_MENU_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "core/ock-station.h"

/* GObject declarations */

#define OCK_TYPE_STATION_CONTEXT_MENU ock_station_context_menu_get_type()

G_DECLARE_FINAL_TYPE(OckStationContextMenu, ock_station_context_menu, \
                     OCK, STATION_CONTEXT_MENU, GtkMenu)

/* Methods */

GtkWidget *ock_station_context_menu_new             (void);
GtkWidget *ock_station_context_menu_new_with_station(OckStation *station);

#endif /* __OVERCOOKED_UI_OCK_STATION_CONTEXT_MENU_H__ */
