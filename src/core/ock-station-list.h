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

#ifndef __OVERCOOKED_CORE_OCK_STATION_LIST_H__
#define __OVERCOOKED_CORE_OCK_STATION_LIST_H__

#include <glib-object.h>

#include "core/ock-station.h"

/* GObject declarations */

#define OCK_TYPE_STATION_LIST ock_station_list_get_type()

G_DECLARE_FINAL_TYPE(OckStationList, ock_station_list, OCK, STATION_LIST, GObject)

/* Data types */

typedef struct _OckStationListIter OckStationListIter;

/* Methods */

OckStationList *ock_station_list_new (void);
void            ock_station_list_load(OckStationList *self);
void            ock_station_list_save(OckStationList *self);

void ock_station_list_prepend      (OckStationList *self, OckStation *station);
void ock_station_list_append       (OckStationList *self, OckStation *station);
void ock_station_list_insert       (OckStationList *self, OckStation *station, gint position);
void ock_station_list_insert_before(OckStationList *self, OckStation *station, OckStation *before);
void ock_station_list_insert_after (OckStationList *self, OckStation *station, OckStation *after);
void ock_station_list_remove       (OckStationList *self, OckStation *station);

void ock_station_list_move       (OckStationList *self, OckStation *station, gint position);
void ock_station_list_move_before(OckStationList *self, OckStation *station, OckStation *before);
void ock_station_list_move_after (OckStationList *self, OckStation *station, OckStation *after);
void ock_station_list_move_first (OckStationList *self, OckStation *station);
void ock_station_list_move_last  (OckStationList *self, OckStation *station);

OckStation *ock_station_list_first(OckStationList *self);
OckStation *ock_station_list_last (OckStationList *self);
OckStation *ock_station_list_prev (OckStationList *self, OckStation *station, gboolean repeat,
                                   gboolean shuffle);
OckStation *ock_station_list_next (OckStationList *self, OckStation *station, gboolean repeat,
                                   gboolean shuffle);

OckStation *ock_station_list_find            (OckStationList *self, OckStation *station);
OckStation *ock_station_list_find_by_name    (OckStationList *self, const gchar *name);
OckStation *ock_station_list_find_by_uri     (OckStationList *self, const gchar *uri);
OckStation *ock_station_list_find_by_uid     (OckStationList *self, const gchar *uid);
OckStation *ock_station_list_find_by_guessing(OckStationList *self, const gchar *string);

/* Properties */

guint ock_station_list_get_length(OckStationList *self);

/* Iterator methods */

OckStationListIter *ock_station_list_iter_new (OckStationList *self);
void                ock_station_list_iter_free(OckStationListIter *iter);
gboolean            ock_station_list_iter_loop(OckStationListIter *iter, OckStation **station);

#endif /* __OVERCOOKED_CORE_OCK_STATION_LIST_H__ */
