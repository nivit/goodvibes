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

#ifndef __GOODVIBES_CORE_GV_PLAYER_H__
#define __GOODVIBES_CORE_GV_PLAYER_H__

#include <glib-object.h>

#include "core/gv-engine.h"
#include "core/gv-metadata.h"
#include "core/gv-station.h"
#include "core/gv-station-list.h"

/* GObject declarations */

#define GV_TYPE_PLAYER gv_player_get_type()

G_DECLARE_FINAL_TYPE(GvPlayer, gv_player, GV, PLAYER, GObject)

/* Data types */

typedef enum {
	GV_PLAYER_STATE_STOPPED,
	GV_PLAYER_STATE_CONNECTING,
	GV_PLAYER_STATE_BUFFERING,
	GV_PLAYER_STATE_PLAYING
} GvPlayerState;

/* Methods */

GvPlayer    *gv_player_new              (GvEngine *engine, GvStationList *station_list);

void          gv_player_go               (GvPlayer *self, const gchar *string_to_play);

void          gv_player_play             (GvPlayer *self);
void          gv_player_stop             (GvPlayer *self);
void          gv_player_toggle           (GvPlayer *self);
gboolean      gv_player_prev             (GvPlayer *self);
gboolean      gv_player_next             (GvPlayer *self);

/* Property accessors */

GvPlayerState gv_player_get_state       (GvPlayer *self);
gboolean       gv_player_get_repeat      (GvPlayer *self);
void           gv_player_set_repeat      (GvPlayer *self, gboolean repeat);
gboolean       gv_player_get_shuffle     (GvPlayer *self);
void           gv_player_set_shuffle     (GvPlayer *self, gboolean shuffle);
gboolean       gv_player_get_autoplay    (GvPlayer *self);
void           gv_player_set_autoplay    (GvPlayer *self, gboolean autoplay);
guint          gv_player_get_volume      (GvPlayer *self);
void           gv_player_set_volume      (GvPlayer *self, guint volume);
void           gv_player_lower_volume    (GvPlayer *self);
void           gv_player_raise_volume    (GvPlayer *self);
gboolean       gv_player_get_mute        (GvPlayer *self);
void           gv_player_set_mute        (GvPlayer *self, gboolean mute);
void           gv_player_toggle_mute     (GvPlayer *self);
GvMetadata   *gv_player_get_metadata    (GvPlayer *self);
void           gv_player_set_metadata    (GvPlayer *self, GvMetadata *metadata);
GvStation    *gv_player_get_station     (GvPlayer *self);
GvStation    *gv_player_get_prev_station(GvPlayer *self);
GvStation    *gv_player_get_next_station(GvPlayer *self);
void           gv_player_set_station     (GvPlayer *self, GvStation *station);
const gchar   *gv_player_get_stream_uri  (GvPlayer *self);

gboolean       gv_player_set_station_by_name    (GvPlayer *self, const gchar *name);
gboolean       gv_player_set_station_by_uri     (GvPlayer *self, const gchar *uri);
gboolean       gv_player_set_station_by_guessing(GvPlayer *self, const gchar *string);

#endif /* __GOODVIBES_CORE_GV_PLAYER_H__ */
