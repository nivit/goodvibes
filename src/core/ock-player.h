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

#ifndef __OVERCOOKED_CORE_OCK_PLAYER_H__
#define __OVERCOOKED_CORE_OCK_PLAYER_H__

#include <glib-object.h>

#include "core/ock-metadata.h"
#include "core/ock-station.h"
#include "core/ock-station-list.h"

/* GObject declarations */

#define OCK_TYPE_PLAYER ock_player_get_type()

G_DECLARE_FINAL_TYPE(OckPlayer, ock_player, OCK, PLAYER, GObject)

/* Data types */

typedef enum {
	OCK_PLAYER_STATE_STOPPED,
	OCK_PLAYER_STATE_CONNECTING,
	OCK_PLAYER_STATE_BUFFERING,
	OCK_PLAYER_STATE_PLAYING
} OckPlayerState;

/* Methods */

OckPlayer    *ock_player_new              (OckStationList *station_list);

void          ock_player_go               (OckPlayer *self, const gchar *string_to_play);

void          ock_player_play             (OckPlayer *self);
void          ock_player_stop             (OckPlayer *self);
void          ock_player_toggle           (OckPlayer *self);
gboolean      ock_player_prev             (OckPlayer *self);
gboolean      ock_player_next             (OckPlayer *self);

/* Property accessors */

OckPlayerState ock_player_get_state       (OckPlayer *self);
gboolean       ock_player_get_repeat      (OckPlayer *self);
void           ock_player_set_repeat      (OckPlayer *self, gboolean repeat);
gboolean       ock_player_get_shuffle     (OckPlayer *self);
void           ock_player_set_shuffle     (OckPlayer *self, gboolean shuffle);
gboolean       ock_player_get_autoplay    (OckPlayer *self);
void           ock_player_set_autoplay    (OckPlayer *self, gboolean autoplay);
guint          ock_player_get_volume      (OckPlayer *self);
void           ock_player_set_volume      (OckPlayer *self, guint volume);
void           ock_player_lower_volume    (OckPlayer *self);
void           ock_player_raise_volume    (OckPlayer *self);
gboolean       ock_player_get_mute        (OckPlayer *self);
void           ock_player_set_mute        (OckPlayer *self, gboolean mute);
void           ock_player_toggle_mute     (OckPlayer *self);
OckMetadata   *ock_player_get_metadata    (OckPlayer *self);
void           ock_player_set_metadata    (OckPlayer *self, OckMetadata *metadata);
OckStation    *ock_player_get_station     (OckPlayer *self);
OckStation    *ock_player_get_prev_station(OckPlayer *self);
OckStation    *ock_player_get_next_station(OckPlayer *self);
void           ock_player_set_station     (OckPlayer *self, OckStation *station);
const gchar   *ock_player_get_stream_uri  (OckPlayer *self);

// TODO I'm not sure, but I think it's something missing in the mpris2 implementation
#if 0
gboolean       ock_player_set_station_by_uid     (OckPlayer *self, const gchar *uid);
#endif
gboolean       ock_player_set_station_by_name    (OckPlayer *self, const gchar *name);
gboolean       ock_player_set_station_by_uri     (OckPlayer *self, const gchar *uri);
gboolean       ock_player_set_station_by_guessing(OckPlayer *self, const gchar *string);

#endif /* __OVERCOOKED_CORE_OCK_PLAYER_H__ */
