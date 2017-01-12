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

#ifndef __GOODVIBES_CORE_GV_STATION_H__
#define __GOODVIBES_CORE_GV_STATION_H__

#include <glib-object.h>

// WISHED Stations should be initially unowned, since they're meant to be added
//        to the station list. But it's a bit slippery to change that now.

/* GObject declarations */

#define GV_TYPE_STATION gv_station_get_type()

G_DECLARE_FINAL_TYPE(GvStation, gv_station, GV, STATION, GObject)

/* Methods */

GvStation  *gv_station_new              (const gchar *name, const gchar *uri);
gchar       *gv_station_make_name        (GvStation *self, gboolean escape);
gboolean     gv_station_download_playlist(GvStation *self);

const gchar *gv_station_get_uid        (GvStation *self);
const gchar *gv_station_get_name       (GvStation *self);
void         gv_station_set_name       (GvStation *self, const gchar *name);
const gchar *gv_station_get_uri        (GvStation *self);
void         gv_station_set_uri        (GvStation *self, const gchar *uri);
const gchar *gv_station_get_name_or_uri(GvStation *self);
GSList      *gv_station_get_stream_uris(GvStation *self);

#endif /* __GOODVIBES_CORE_GV_STATION_H__ */
