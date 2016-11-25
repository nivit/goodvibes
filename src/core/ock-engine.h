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

#ifndef __OVERCOOKED_CORE_OCK_ENGINE_H__
#define __OVERCOOKED_CORE_OCK_ENGINE_H__

#include <glib.h>
#include <glib-object.h>

#include "core/ock-metadata.h"

/* GObject declarations */

#define OCK_TYPE_ENGINE ock_engine_get_type()

G_DECLARE_FINAL_TYPE(OckEngine, ock_engine, OCK, ENGINE, GObject)

/* Data types */

typedef enum {
	OCK_ENGINE_STATE_STOPPED = 0,
	OCK_ENGINE_STATE_BUFFERING,
	OCK_ENGINE_STATE_PLAYING
} OckEngineState;

typedef enum {
	OCK_ENGINE_ERROR_STREAM_UNDEFINED,
	OCK_ENGINE_ERROR_STREAM_UNAVAILABLE,
	OCK_ENGINE_ERROR_STREAM_FORMAT_UNRECOGNIZED,
	OCK_ENGINE_ERROR_SERVER_NAME_UNRESOLVED
} OckEngineError;

/* Methods */

OckEngine *ock_engine_new    (void);
void       ock_engine_play   (OckEngine *self);
void       ock_engine_stop   (OckEngine *self);

/* Property accessors */

OckEngineState  ock_engine_get_state     (OckEngine *self);
gdouble         ock_engine_get_volume    (OckEngine *self);
void            ock_engine_set_volume    (OckEngine *self, gdouble volume);
gboolean        ock_engine_get_mute      (OckEngine *self);
void            ock_engine_set_mute      (OckEngine *self, gboolean mute);
const gchar    *ock_engine_get_stream_uri(OckEngine *self);
void            ock_engine_set_stream_uri(OckEngine *self, const gchar *uri);
OckMetadata    *ock_engine_get_metadata  (OckEngine *self);

#endif /* __OVERCOOKED_CORE_OCK_ENGINE_H__ */
