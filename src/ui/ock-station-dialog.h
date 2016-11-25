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

#ifndef __OVERCOOKED_UI_OCK_STATION_DIALOG_H__
#define __OVERCOOKED_UI_OCK_STATION_DIALOG_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "core/ock-station.h"

/* GObject declarations */

#define OCK_TYPE_STATION_DIALOG ock_station_dialog_get_type()

G_DECLARE_FINAL_TYPE(OckStationDialog, ock_station_dialog, OCK, STATION_DIALOG, GtkDialog)

/* Methods */

GtkWidget  *ock_station_dialog_new         (void);
void        ock_station_dialog_populate    (OckStationDialog *self, OckStation *station);
void        ock_station_dialog_retrieve    (OckStationDialog *self, OckStation *station);
OckStation *ock_station_dialog_retrieve_new(OckStationDialog *self);

#endif /* __OVERCOOKED_UI_OCK_STATION_DIALOG_H__ */
