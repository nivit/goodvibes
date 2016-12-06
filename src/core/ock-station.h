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

#ifndef __OVERCOOKED_CORE_OCK_STATION_H__
#define __OVERCOOKED_CORE_OCK_STATION_H__

#include <glib-object.h>

// WISHED Stations should be initially unowned, since they're meant to be added
//        to the station list. But it's a bit slippery to change that now.

/* GObject declarations */

#define OCK_TYPE_STATION ock_station_get_type()

G_DECLARE_FINAL_TYPE(OckStation, ock_station, OCK, STATION, GObject)

/* Methods */

OckStation  *ock_station_new              (const gchar *name, const gchar *uri);
gchar       *ock_station_make_name        (OckStation *self, gboolean escape);
gchar       *ock_station_to_string        (OckStation *self);
gboolean     ock_station_download_playlist(OckStation *self);

const gchar *ock_station_get_uid        (OckStation *self);
const gchar *ock_station_get_name       (OckStation *self);
void         ock_station_set_name       (OckStation *self, const gchar *name);
const gchar *ock_station_get_uri        (OckStation *self);
void         ock_station_set_uri        (OckStation *self, const gchar *uri);
const gchar *ock_station_get_name_or_uri(OckStation *self);
GSList      *ock_station_get_stream_uris(OckStation *self);

#endif /* __OVERCOOKED_CORE_OCK_STATION_H__ */
