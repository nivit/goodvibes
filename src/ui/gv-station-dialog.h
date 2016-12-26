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

#ifndef __GOODVIBES_UI_GV_STATION_DIALOG_H__
#define __GOODVIBES_UI_GV_STATION_DIALOG_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "core/gv-station.h"

/* GObject declarations */

#define GV_TYPE_STATION_DIALOG gv_station_dialog_get_type()

G_DECLARE_FINAL_TYPE(GvStationDialog, gv_station_dialog, GV, STATION_DIALOG, GtkDialog)

/* Methods */

GtkWidget  *gv_station_dialog_new         (void);
void        gv_station_dialog_populate    (GvStationDialog *self, GvStation *station);
void        gv_station_dialog_retrieve    (GvStationDialog *self, GvStation *station);
GvStation *gv_station_dialog_retrieve_new(GvStationDialog *self);

#endif /* __GOODVIBES_UI_GV_STATION_DIALOG_H__ */
