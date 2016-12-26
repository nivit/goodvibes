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

#ifndef __GOODVIBES_CORE_GV_STATION_LIST_H__
#define __GOODVIBES_CORE_GV_STATION_LIST_H__

#include <glib-object.h>

#include "core/gv-station.h"

/* GObject declarations */

#define GV_TYPE_STATION_LIST gv_station_list_get_type()

G_DECLARE_FINAL_TYPE(GvStationList, gv_station_list, GV, STATION_LIST, GObject)

/* Data types */

typedef struct _GvStationListIter GvStationListIter;

/* Methods */

GvStationList *gv_station_list_new (void);
void            gv_station_list_load(GvStationList *self);
void            gv_station_list_save(GvStationList *self);

void gv_station_list_prepend      (GvStationList *self, GvStation *station);
void gv_station_list_append       (GvStationList *self, GvStation *station);
void gv_station_list_insert       (GvStationList *self, GvStation *station, gint position);
void gv_station_list_insert_before(GvStationList *self, GvStation *station, GvStation *before);
void gv_station_list_insert_after (GvStationList *self, GvStation *station, GvStation *after);
void gv_station_list_remove       (GvStationList *self, GvStation *station);

void gv_station_list_move       (GvStationList *self, GvStation *station, gint position);
void gv_station_list_move_before(GvStationList *self, GvStation *station, GvStation *before);
void gv_station_list_move_after (GvStationList *self, GvStation *station, GvStation *after);
void gv_station_list_move_first (GvStationList *self, GvStation *station);
void gv_station_list_move_last  (GvStationList *self, GvStation *station);

GvStation *gv_station_list_first(GvStationList *self);
GvStation *gv_station_list_last (GvStationList *self);
GvStation *gv_station_list_prev (GvStationList *self, GvStation *station, gboolean repeat,
                                 gboolean shuffle);
GvStation *gv_station_list_next (GvStationList *self, GvStation *station, gboolean repeat,
                                 gboolean shuffle);

GvStation *gv_station_list_find            (GvStationList *self, GvStation *station);
GvStation *gv_station_list_find_by_name    (GvStationList *self, const gchar *name);
GvStation *gv_station_list_find_by_uri     (GvStationList *self, const gchar *uri);
GvStation *gv_station_list_find_by_uid     (GvStationList *self, const gchar *uid);
GvStation *gv_station_list_find_by_guessing(GvStationList *self, const gchar *string);

/* Properties */

guint gv_station_list_get_length(GvStationList *self);

/* Iterator methods */

GvStationListIter *gv_station_list_iter_new (GvStationList *self);
void                gv_station_list_iter_free(GvStationListIter *iter);
gboolean            gv_station_list_iter_loop(GvStationListIter *iter, GvStation **station);

#endif /* __GOODVIBES_CORE_GV_STATION_LIST_H__ */
