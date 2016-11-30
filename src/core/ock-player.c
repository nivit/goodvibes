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

#include <math.h>
#include <glib.h>
#include <glib-object.h>

#include "additions/glib-object.h"

#include "libgszn/gszn.h"

#include "framework/log.h"
#include "framework/uri-schemes.h"
#include "framework/ock-framework.h"

#include "core/ock-engine.h"
#include "core/ock-core-enum-types.h"
#include "core/ock-metadata.h"
#include "core/ock-station.h"
#include "core/ock-station-list.h"

#include "core/ock-player.h"

/*
 * Properties
 */

#define DEFAULT_VOLUME   100
#define DEFAULT_MUTE     FALSE
#define DEFAULT_REPEAT   TRUE
#define DEFAULT_SHUFFLE  FALSE
#define DEFAULT_AUTOPLAY FALSE

enum {
	/* Reserved */
	PROP_0,
	/* Construct properties */
	PROP_STATION_LIST,
	/* Properties */
	PROP_STATE,
	PROP_VOLUME,
	PROP_MUTE,
	PROP_REPEAT,
	PROP_SHUFFLE,
	PROP_AUTOPLAY,
	PROP_METADATA,
	PROP_STATION,
	PROP_STATION_URI, /* For serialization only */
	PROP_PREV_STATION,
	PROP_NEXT_STATION,
	PROP_STREAM_URI,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * GObject definitions
 */

typedef enum {
	OCK_PLAYER_WISH_TO_STOP,
	OCK_PLAYER_WISH_TO_PLAY,
} OckPlayerWish;

struct _OckPlayerPrivate {
	/* Properties */
	OckPlayerState  state;
	guint           volume;
	gboolean        mute;
	gboolean        repeat;
	gboolean        shuffle;
	gboolean        autoplay;
	/* Engine */
	OckEngine      *engine;
	/* Station list */
	OckStationList *station_list;
	/* Current station */
	OckStation     *station;
	OckMetadata    *metadata;
	/* Wished state */
	OckPlayerWish   wish;
};

typedef struct _OckPlayerPrivate OckPlayerPrivate;

struct _OckPlayer {
	/* Parent instance structure */
	GObject           parent_instance;
	/* Private data */
	OckPlayerPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(OckPlayer, ock_player, G_TYPE_OBJECT)

/*
 * Signal handlers
 */

static void ock_player_set_state(OckPlayer *self, OckPlayerState value);

static void
on_station_notify(OckStation *station,
                  GParamSpec *pspec,
                  OckPlayer  *self)
{
	OckPlayerPrivate *priv = self->priv;
	const gchar *property_name = g_param_spec_get_name(pspec);

	TRACE("%p, %s, %p", station, property_name, self);

	g_assert(station == priv->station);

	if (!g_strcmp0(property_name, "stream-uris")) {
		WARNING("Stream uri changed");

		/* Check if there are some streams, and start playing if needed */
		if (ock_station_get_stream_uris(station))
			if (priv->wish == OCK_PLAYER_WISH_TO_PLAY)
				ock_player_play(self);
	}

	/* In any case, we notify if something was changed in the station */
	g_object_notify(G_OBJECT(self), "station");
}

static void
on_engine_notify(OckEngine  *engine,
                 GParamSpec *pspec,
                 OckPlayer  *self)
{
	OckPlayerPrivate *priv = self->priv;
	const gchar *property_name = g_param_spec_get_name(pspec);

	TRACE("%p, %s, %p", engine, property_name, self);

	if (!g_strcmp0(property_name, "state")) {
		OckEngineState engine_state;
		OckPlayerState player_state;

		engine_state = ock_engine_get_state(priv->engine);

		/* Map engine state to player state - trivial */
		switch (engine_state) {
		case OCK_ENGINE_STATE_STOPPED:
			player_state = OCK_PLAYER_STATE_STOPPED;
			break;
		case OCK_ENGINE_STATE_CONNECTING:
			player_state = OCK_PLAYER_STATE_CONNECTING;
			break;
		case OCK_ENGINE_STATE_BUFFERING:
			player_state = OCK_PLAYER_STATE_BUFFERING;
			break;
		case OCK_ENGINE_STATE_PLAYING:
			player_state = OCK_PLAYER_STATE_PLAYING;
			break;
		default:
			ERROR("Unhandled engine state: %d", engine_state);
			/* Program execution stops here */
			break;
		}

		/* Set state */
		ock_player_set_state(self, player_state);

	} else if (!g_strcmp0(property_name, "metadata")) {
		/* Metadata was updated, let's set it in our properties */
		OckMetadata *metadata;

		metadata = ock_engine_get_metadata(engine);
		ock_player_set_metadata(self, metadata);
	}
}

/*
 * Property accessors
 */

static void
ock_player_set_station_list(OckPlayer *self, OckStationList *station_list)
{
	OckPlayerPrivate *priv = self->priv;

	/* This is a construct-only property */
	g_assert(priv->station_list == NULL);
	g_assert(station_list != NULL);
	priv->station_list = g_object_ref(station_list);
}

OckPlayerState
ock_player_get_state(OckPlayer *self)
{
	return self->priv->state;
}

static void
ock_player_set_state(OckPlayer *self, OckPlayerState state)
{
	OckPlayerPrivate *priv = self->priv;

	if (priv->state == state)
		return;

	priv->state = state;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_STATE]);
}

guint
ock_player_get_volume(OckPlayer *self)
{
	return self->priv->volume;
}

void
ock_player_set_volume(OckPlayer *self, guint volume)
{
	OckPlayerPrivate *priv = self->priv;
	gdouble engine_volume;

	if (volume > 100)
		volume = 100;

	if (priv->volume == volume)
		return;

	priv->volume = volume;

	engine_volume = (gdouble) volume / 100.0;
	ock_engine_set_volume(priv->engine, engine_volume);

	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_VOLUME]);
}

void
ock_player_lower_volume(OckPlayer *self)
{
	guint volume;
	guint step = 5;

	volume = ock_player_get_volume(self);
	volume = volume > step ? volume - step : 0;

	ock_player_set_volume(self, volume);
}

void
ock_player_raise_volume(OckPlayer *self)
{
	guint volume;
	guint step = 5;

	volume = ock_player_get_volume(self);
	volume = volume < 100 - step ? volume + step : 100;

	ock_player_set_volume(self, volume);
}

gboolean
ock_player_get_mute(OckPlayer *self)
{
	return self->priv->mute;
}

void
ock_player_set_mute(OckPlayer *self, gboolean mute)
{
	OckPlayerPrivate *priv = self->priv;

	if (priv->mute == mute)
		return;

	priv->mute = mute;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MUTE]);
}

void
ock_player_toggle_mute(OckPlayer *self)
{
	gboolean mute;

	mute = ock_player_get_mute(self);
	ock_player_set_mute(self, !mute);
}

gboolean
ock_player_get_repeat(OckPlayer *self)
{
	return self->priv->repeat;
}

void
ock_player_set_repeat(OckPlayer *self, gboolean repeat)
{
	OckPlayerPrivate *priv = self->priv;

	if (priv->repeat == repeat)
		return;

	priv->repeat = repeat;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_REPEAT]);
}

gboolean
ock_player_get_shuffle(OckPlayer *self)
{
	return self->priv->shuffle;
}

void
ock_player_set_shuffle(OckPlayer *self, gboolean shuffle)
{
	OckPlayerPrivate *priv = self->priv;

	if (priv->shuffle == shuffle)
		return;

	priv->shuffle = shuffle;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_SHUFFLE]);
}

gboolean
ock_player_get_autoplay(OckPlayer *self)
{
	return self->priv->autoplay;
}

void
ock_player_set_autoplay(OckPlayer *self, gboolean autoplay)
{
	OckPlayerPrivate *priv = self->priv;

	if (priv->autoplay == autoplay)
		return;

	priv->autoplay = autoplay;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_AUTOPLAY]);
}

OckMetadata *
ock_player_get_metadata(OckPlayer *self)
{
	return self->priv->metadata;
}

void
ock_player_set_metadata(OckPlayer *self, OckMetadata *metadata)
{
	OckPlayerPrivate *priv = self->priv;

	if (g_set_object(&priv->metadata, metadata))
		g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_METADATA]);
}

OckStation *
ock_player_get_station(OckPlayer *self)
{
	return self->priv->station;
}

OckStation *
ock_player_get_prev_station(OckPlayer *self)
{
	OckPlayerPrivate *priv = self->priv;

	return ock_station_list_prev(priv->station_list, priv->station,
	                             priv->repeat, priv->shuffle);
}

OckStation *
ock_player_get_next_station(OckPlayer *self)
{
	OckPlayerPrivate *priv = self->priv;

	return ock_station_list_next(priv->station_list, priv->station,
	                             priv->repeat, priv->shuffle);
}

static const gchar *
ock_player_get_station_uri(OckPlayer *self)
{
	OckStation *station;
	const gchar *station_uri;

	station = ock_player_get_station(self);
	if (station == NULL)
		return NULL;

	station_uri = ock_station_get_uri(station);
	return station_uri;
}

void
ock_player_set_station(OckPlayer *self, OckStation *station)
{
	OckPlayerPrivate *priv = self->priv;

	if (station == priv->station)
		return;

	if (priv->station) {
		g_signal_handlers_disconnect_by_data(priv->station, self);
		g_object_unref(priv->station);
		priv->station = NULL;
	}

	if (station) {
		priv->station = g_object_ref(station);
		g_signal_connect(priv->station, "notify", G_CALLBACK(on_station_notify), self);
	}

	ock_player_set_metadata(self, NULL);

	g_object_notify(G_OBJECT(self), "station");
	g_object_notify(G_OBJECT(self), "station-uri");

	INFO("Station set to '%s'", station ? ock_station_get_name_or_uri(station) : NULL);
}

gboolean
ock_player_set_station_by_name(OckPlayer *self, const gchar *name)
{
	OckPlayerPrivate *priv = self->priv;
	OckStation *station = NULL;

	station = ock_station_list_find_by_name(priv->station_list, name);
	if (station == NULL) {
		DEBUG("Station name '%s' not found in station list", name);
		return FALSE;
	}

	ock_player_set_station(self, station);
	return TRUE;
}

gboolean
ock_player_set_station_by_uri(OckPlayer *self, const gchar *uri)
{
	OckPlayerPrivate *priv = self->priv;
	OckStation *station = NULL;

	/* Look for the station in the station list */
	station = ock_station_list_find_by_uri(priv->station_list, uri);
	if (station == NULL) {
		DEBUG("Station uri '%s' not found in station list", uri);
		return FALSE;
	}

	ock_player_set_station(self, station);
	return TRUE;
}

gboolean
ock_player_set_station_by_guessing(OckPlayer *self, const gchar *string)
{
	OckPlayerPrivate *priv = self->priv;
	OckStation *station = NULL;

	station = ock_station_list_find_by_guessing(priv->station_list, string);
	if (station == NULL) {
		DEBUG("'%s' not found in station list", string);
		return FALSE;
	}

	ock_player_set_station(self, station);
	return TRUE;
}

const gchar *
ock_player_get_stream_uri(OckPlayer *self)
{
	OckPlayerPrivate *priv = self->priv;

	return ock_engine_get_stream_uri(priv->engine);
}

static void
ock_player_get_property(GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
	OckPlayer *self = OCK_PLAYER(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_STATE:
		g_value_set_enum(value, ock_player_get_state(self));
		break;
	case PROP_VOLUME:
		g_value_set_uint(value, ock_player_get_volume(self));
		break;
	case PROP_MUTE:
		g_value_set_boolean(value, ock_player_get_mute(self));
		break;
	case PROP_REPEAT:
		g_value_set_boolean(value, ock_player_get_repeat(self));
		break;
	case PROP_SHUFFLE:
		g_value_set_boolean(value, ock_player_get_shuffle(self));
		break;
	case PROP_AUTOPLAY:
		g_value_set_boolean(value, ock_player_get_autoplay(self));
		break;
	case PROP_METADATA:
		g_value_set_object(value, ock_player_get_metadata(self));
		break;
	case PROP_STATION:
		g_value_set_object(value, ock_player_get_station(self));
		break;
	case PROP_STATION_URI:
		g_value_set_string(value, ock_player_get_station_uri(self));
		break;
	case PROP_PREV_STATION:
		g_value_set_object(value, ock_player_get_prev_station(self));
		break;
	case PROP_NEXT_STATION:
		g_value_set_object(value, ock_player_get_next_station(self));
		break;
	case PROP_STREAM_URI:
		g_value_set_string(value, ock_player_get_stream_uri(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
ock_player_set_property(GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
	OckPlayer *self = OCK_PLAYER(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_STATION_LIST:
		ock_player_set_station_list(self, g_value_get_object(value));
		break;
	case PROP_VOLUME:
		ock_player_set_volume(self, g_value_get_uint(value));
		break;
	case PROP_MUTE:
		ock_player_set_mute(self, g_value_get_boolean(value));
		break;
	case PROP_REPEAT:
		ock_player_set_repeat(self, g_value_get_boolean(value));
		break;
	case PROP_SHUFFLE:
		ock_player_set_shuffle(self, g_value_get_boolean(value));
		break;
	case PROP_AUTOPLAY:
		ock_player_set_autoplay(self, g_value_get_boolean(value));
		break;
	case PROP_METADATA:
		ock_player_set_metadata(self, g_value_get_object(value));
		break;
	case PROP_STATION:
		ock_player_set_station(self, g_value_get_object(value));
		break;
	case PROP_STATION_URI:
		ock_player_set_station_by_uri(self, g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Public methods
 */

void
ock_player_stop(OckPlayer *self)
{
	OckPlayerPrivate *priv = self->priv;

	/* To remember what we're doing */
	priv->wish = OCK_PLAYER_WISH_TO_STOP;

	/* Stop playing */
	ock_engine_stop(priv->engine);
}

void
ock_player_play(OckPlayer *self)
{
	OckPlayerPrivate *priv = self->priv;
	OckStation *station;
	GSList *uris;

	/* If no station is set yet, take the first from the station list */
	if (priv->station == NULL) {
		OckStation *first_station;

		first_station = ock_station_list_first(priv->station_list);
		ock_player_set_station(self, first_station);
	}

	/* If there's still no station, return */
	station = priv->station;
	if (station == NULL)
		return;

	/* To remember what we're doing */
	priv->wish = OCK_PLAYER_WISH_TO_PLAY;

	/* Stop playing */
	ock_engine_stop(priv->engine);

	/* Get station data */
	uris = ock_station_get_stream_uris(station);

	/* If there's no uris, that probably means that the station uri
	 * points to a playlist, and we need to download it.
	 */
	if (uris == NULL) {
		/* Download the playlist that contains the stream uris */
		if (!ock_station_download_playlist(station))
			WARNING("Can't download playlist");

		/* Downloading a playlist is an asynchronous operation.
		 * We have nothing left to do here.
		 */
		return;
	} else {
		const gchar *first_uri;

		/* Play the first uri */
		first_uri = (gchar *) uris->data;
		ock_engine_set_stream_uri(priv->engine, first_uri);
		ock_engine_play(priv->engine);
	}
}

gboolean
ock_player_next(OckPlayer *self)
{
	OckPlayerPrivate *priv = self->priv;
	OckStation *station;

	station = ock_player_get_next_station(self);

	if (station == NULL)
		return FALSE;

	ock_player_set_station(self, station);

	if (priv->wish == OCK_PLAYER_WISH_TO_PLAY)
		ock_player_play(self);

	return TRUE;
}

gboolean
ock_player_prev(OckPlayer *self)
{
	OckPlayerPrivate *priv = self->priv;
	OckStation *station;

	station = ock_player_get_prev_station(self);

	if (station == NULL)
		return FALSE;

	ock_player_set_station(self, station);

	if (priv->wish == OCK_PLAYER_WISH_TO_PLAY)
		ock_player_play(self);

	return TRUE;
}

void
ock_player_toggle(OckPlayer *self)
{
	OckPlayerPrivate *priv = self->priv;

	switch (priv->wish) {
	case OCK_PLAYER_WISH_TO_STOP:
		ock_player_play(self);
		break;
	case OCK_PLAYER_WISH_TO_PLAY:
		ock_player_stop(self);
		break;
	default:
		ERROR("Invalid wish: %d", priv->wish);
		/* Program execution stops here */
		break;
	}
}

void
ock_player_go(OckPlayer *self, const gchar *string_to_play)
{
	OckPlayerPrivate *priv = self->priv;

	/* If we have no argument, we rely on 'autoplay' */
	if (string_to_play == NULL) {
		if (priv->autoplay) {
			INFO("Autoplay is enabled, let's play");
			ock_player_play(self);
		}
		return;
	}

	/* If we have an argument, it might be anything.
	 * At first, check if we find it in the station list.
	 */
	if (ock_player_set_station_by_guessing(self, string_to_play)) {
		INFO("'%s' found in station list, let's play", string_to_play);
		ock_player_play(self);
		return;
	}

	/* Otherwise, if it's a valid uri, try to play it */
	if (is_uri_scheme_supported(string_to_play)) {
		OckStation *station;

		station = ock_station_new(NULL, string_to_play);
		ock_player_set_station(self, station);
		g_object_unref(station);

		INFO("'%s' is a valid uri, let's play", string_to_play);
		ock_player_play(self);
		return;
	}

	/* That looks like an invalid string then */
	INFO("'%s' is neither a known station or a valid uri", string_to_play);

	// TODO Report that error ?
}

OckPlayer *
ock_player_new(OckStationList *station_list)
{
	return g_object_new(OCK_TYPE_PLAYER,
	                    "station-list", station_list,
	                    NULL);
}

/*
 * GObject methods
 */

static void
ock_player_finalize(GObject *object)
{
	OckPlayer *self = OCK_PLAYER(object);
	OckPlayerPrivate *priv = self->priv;

	TRACE("%p", object);

	/* Unref the metadata */
	if (priv->metadata)
		g_object_unref(priv->metadata);

	/* Unref the current station */
	if (priv->station) {
		g_signal_handlers_disconnect_by_data(priv->station, self);
		g_object_unref(priv->station);
	}

	/* Unref the station list */
	g_object_unref(priv->station_list);

	/* Unref the engine */
	g_signal_handlers_disconnect_by_data(priv->engine, self);
	g_object_unref(priv->engine);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_player, object);
}

static void
ock_player_constructed(GObject *object)
{
	OckPlayer *self = OCK_PLAYER(object);
	OckPlayerPrivate *priv = self->priv;

	TRACE("%p", object);

	/* Initialize properties */
	priv->volume   = DEFAULT_VOLUME;
	priv->mute     = DEFAULT_MUTE;
	priv->repeat   = DEFAULT_REPEAT;
	priv->shuffle  = DEFAULT_SHUFFLE;
	priv->autoplay = DEFAULT_AUTOPLAY;
	priv->station  = NULL;

	/* Create engine */
	priv->engine = ock_engine_new();
	g_signal_connect(priv->engine, "notify", G_CALLBACK(on_engine_notify), self);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_player, object);
}

static void
ock_player_init(OckPlayer *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_player_get_instance_private(self);
}

static void
ock_player_class_init(OckPlayerClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_player_finalize;
	object_class->constructed = ock_player_constructed;

	/* Properties */
	object_class->get_property = ock_player_get_property;
	object_class->set_property = ock_player_set_property;

	properties[PROP_STATION_LIST] =
	        g_param_spec_object("station-list", "Station list", NULL,
	                            OCK_TYPE_STATION_LIST,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_WRITABLE |
	                            G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_STATE] =
	        g_param_spec_enum("state", "Playback State", NULL,
	                          OCK_PLAYER_STATE_ENUM_TYPE,
	                          OCK_PLAYER_STATE_STOPPED,
	                          OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	properties[PROP_VOLUME] =
	        g_param_spec_uint("volume", "Volume In Percent", NULL,
	                          0, 100, DEFAULT_VOLUME,
	                          OCK_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                          G_PARAM_READWRITE);

	properties[PROP_MUTE] =
	        g_param_spec_boolean("mute", "Mute", NULL,
	                             DEFAULT_MUTE,
	                             OCK_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                             G_PARAM_READWRITE);

	properties[PROP_REPEAT] =
	        g_param_spec_boolean("repeat", "Repeat", NULL,
	                             DEFAULT_REPEAT,
	                             OCK_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                             G_PARAM_READWRITE);

	properties[PROP_SHUFFLE] =
	        g_param_spec_boolean("shuffle", "Shuffle", NULL,
	                             DEFAULT_SHUFFLE,
	                             OCK_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                             G_PARAM_READWRITE);

	properties[PROP_AUTOPLAY] =
	        g_param_spec_boolean("autoplay", "Autoplay On Startup", NULL,
	                             DEFAULT_AUTOPLAY,
	                             OCK_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                             G_PARAM_READWRITE);

	properties[PROP_METADATA] =
	        g_param_spec_object("metadata", "Current Metadata", NULL,
	                            OCK_TYPE_METADATA,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_STATION] =
	        g_param_spec_object("station", "Current Station", NULL,
	                            OCK_TYPE_STATION,
	                            OCK_PARAM_DEFAULT_FLAGS |
	                            G_PARAM_READWRITE);

	properties[PROP_STATION_URI] =
	        g_param_spec_string("station-uri", "Current Station Uri",
	                            "This is a workaround for serialization, because we "
	                            "can't serialize the 'station' property (GObject type)",
	                            NULL,
	                            OCK_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                            G_PARAM_READWRITE);

	properties[PROP_PREV_STATION] =
	        g_param_spec_object("prev", "Previous Station", NULL,
	                            OCK_TYPE_STATION,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	properties[PROP_NEXT_STATION] =
	        g_param_spec_object("next", "Next Station", NULL,
	                            OCK_TYPE_STATION,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	properties[PROP_STREAM_URI] =
	        g_param_spec_string("stream-uri", "Curent Stream Uri", NULL, NULL,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	g_object_class_install_properties(object_class, PROP_N, properties);
}
