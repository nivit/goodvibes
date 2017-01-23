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

#include <math.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "additions/glib-object.h"

#include "libgszn/gszn.h"

#include "framework/gv-framework.h"

#include "core/gv-engine.h"
#include "core/gv-core-enum-types.h"
#include "core/gv-core.h"
#include "core/gv-metadata.h"
#include "core/gv-station.h"
#include "core/gv-station-list.h"

#include "core/gv-player.h"

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
	PROP_ENGINE,
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
	GV_PLAYER_WISH_TO_STOP,
	GV_PLAYER_WISH_TO_PLAY,
} GvPlayerWish;

struct _GvPlayerPrivate {
	/* Construct-only properties */
	GvEngine      *engine;
	GvStationList *station_list;
	/* Properties */
	GvPlayerState  state;
	guint           volume;
	gboolean        mute;
	gboolean        repeat;
	gboolean        shuffle;
	gboolean        autoplay;
	/* Current station */
	GvStation     *station;
	GvMetadata    *metadata;
	/* Wished state */
	GvPlayerWish   wish;
};

typedef struct _GvPlayerPrivate GvPlayerPrivate;

struct _GvPlayer {
	/* Parent instance structure */
	GObject           parent_instance;
	/* Private data */
	GvPlayerPrivate *priv;
};

G_DEFINE_TYPE_WITH_CODE(GvPlayer, gv_player, G_TYPE_OBJECT,
                        G_ADD_PRIVATE(GvPlayer)
                        G_IMPLEMENT_INTERFACE(GV_TYPE_ERRORABLE, NULL))

/*
 * Signal handlers
 */

static void gv_player_set_state(GvPlayer *self, GvPlayerState value);

static void
on_station_notify(GvStation *station,
                  GParamSpec *pspec,
                  GvPlayer  *self)
{
	GvPlayerPrivate *priv = self->priv;
	const gchar *property_name = g_param_spec_get_name(pspec);

	TRACE("%p, %s, %p", station, property_name, self);

	g_assert(station == priv->station);

	if (!g_strcmp0(property_name, "stream-uris")) {
		DEBUG("Station %p: stream uris have changed", station);

		/* Check if there are some streams, and start playing if needed */
		if (gv_station_get_stream_uris(station))
			if (priv->wish == GV_PLAYER_WISH_TO_PLAY)
				gv_player_play(self);
	}

	/* In any case, we notify if something was changed in the station */
	g_object_notify(G_OBJECT(self), "station");
}

static void
on_engine_notify(GvEngine  *engine,
                 GParamSpec *pspec,
                 GvPlayer  *self)
{
	GvPlayerPrivate *priv = self->priv;
	const gchar *property_name = g_param_spec_get_name(pspec);

	TRACE("%p, %s, %p", engine, property_name, self);

	if (!g_strcmp0(property_name, "state")) {
		GvEngineState engine_state;
		GvPlayerState player_state;

		engine_state = gv_engine_get_state(priv->engine);

		/* Map engine state to player state - trivial */
		switch (engine_state) {
		case GV_ENGINE_STATE_STOPPED:
			player_state = GV_PLAYER_STATE_STOPPED;
			break;
		case GV_ENGINE_STATE_CONNECTING:
			player_state = GV_PLAYER_STATE_CONNECTING;
			break;
		case GV_ENGINE_STATE_BUFFERING:
			player_state = GV_PLAYER_STATE_BUFFERING;
			break;
		case GV_ENGINE_STATE_PLAYING:
			player_state = GV_PLAYER_STATE_PLAYING;
			break;
		default:
			ERROR("Unhandled engine state: %d", engine_state);
			/* Program execution stops here */
			break;
		}

		/* Set state */
		gv_player_set_state(self, player_state);

	} else if (!g_strcmp0(property_name, "metadata")) {
		/* Metadata was updated, let's set it in our properties */
		GvMetadata *metadata;

		metadata = gv_engine_get_metadata(engine);
		gv_player_set_metadata(self, metadata);
	}
}

static void
on_engine_error(GvEngine *engine G_GNUC_UNUSED,
                const gchar *error_string G_GNUC_UNUSED,
                GvPlayer *self)
{
	/* Whatever the error, just stop */
	gv_player_stop(self);
}

/*
 * Property accessors
 */

static void
gv_player_set_engine(GvPlayer *self, GvEngine *engine)
{
	GvPlayerPrivate *priv = self->priv;

	/* This is a construct-only property */
	g_assert_null(priv->engine);
	g_assert_nonnull(engine);
	priv->engine = g_object_ref(engine);
	g_signal_connect(priv->engine, "notify", G_CALLBACK(on_engine_notify), self);
	g_signal_connect(priv->engine, "error", G_CALLBACK(on_engine_error), self);
}

static void
gv_player_set_station_list(GvPlayer *self, GvStationList *station_list)
{
	GvPlayerPrivate *priv = self->priv;

	/* This is a construct-only property */
	g_assert_null(priv->station_list);
	g_assert_nonnull(station_list);
	priv->station_list = g_object_ref(station_list);
}

GvPlayerState
gv_player_get_state(GvPlayer *self)
{
	return self->priv->state;
}

static void
gv_player_set_state(GvPlayer *self, GvPlayerState state)
{
	GvPlayerPrivate *priv = self->priv;

	if (priv->state == state)
		return;

	priv->state = state;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_STATE]);
}

guint
gv_player_get_volume(GvPlayer *self)
{
	return self->priv->volume;
}

void
gv_player_set_volume(GvPlayer *self, guint volume)
{
	GvPlayerPrivate *priv = self->priv;
	gdouble engine_volume;

	if (volume > 100)
		volume = 100;

	if (priv->volume == volume)
		return;

	priv->volume = volume;

	engine_volume = (gdouble) volume / 100.0;
	gv_engine_set_volume(priv->engine, engine_volume);

	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_VOLUME]);
}

void
gv_player_lower_volume(GvPlayer *self)
{
	guint volume;
	guint step = 5;

	volume = gv_player_get_volume(self);
	volume = volume > step ? volume - step : 0;

	gv_player_set_volume(self, volume);
}

void
gv_player_raise_volume(GvPlayer *self)
{
	guint volume;
	guint step = 5;

	volume = gv_player_get_volume(self);
	volume = volume < 100 - step ? volume + step : 100;

	gv_player_set_volume(self, volume);
}

gboolean
gv_player_get_mute(GvPlayer *self)
{
	return self->priv->mute;
}

void
gv_player_set_mute(GvPlayer *self, gboolean mute)
{
	GvPlayerPrivate *priv = self->priv;

	if (priv->mute == mute)
		return;

	priv->mute = mute;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MUTE]);
}

void
gv_player_toggle_mute(GvPlayer *self)
{
	gboolean mute;

	mute = gv_player_get_mute(self);
	gv_player_set_mute(self, !mute);
}

gboolean
gv_player_get_repeat(GvPlayer *self)
{
	return self->priv->repeat;
}

void
gv_player_set_repeat(GvPlayer *self, gboolean repeat)
{
	GvPlayerPrivate *priv = self->priv;

	if (priv->repeat == repeat)
		return;

	priv->repeat = repeat;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_REPEAT]);
}

gboolean
gv_player_get_shuffle(GvPlayer *self)
{
	return self->priv->shuffle;
}

void
gv_player_set_shuffle(GvPlayer *self, gboolean shuffle)
{
	GvPlayerPrivate *priv = self->priv;

	if (priv->shuffle == shuffle)
		return;

	priv->shuffle = shuffle;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_SHUFFLE]);
}

gboolean
gv_player_get_autoplay(GvPlayer *self)
{
	return self->priv->autoplay;
}

void
gv_player_set_autoplay(GvPlayer *self, gboolean autoplay)
{
	GvPlayerPrivate *priv = self->priv;

	if (priv->autoplay == autoplay)
		return;

	priv->autoplay = autoplay;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_AUTOPLAY]);
}

GvMetadata *
gv_player_get_metadata(GvPlayer *self)
{
	return self->priv->metadata;
}

void
gv_player_set_metadata(GvPlayer *self, GvMetadata *metadata)
{
	GvPlayerPrivate *priv = self->priv;

	if (g_set_object(&priv->metadata, metadata))
		g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_METADATA]);
}

GvStation *
gv_player_get_station(GvPlayer *self)
{
	return self->priv->station;
}

GvStation *
gv_player_get_prev_station(GvPlayer *self)
{
	GvPlayerPrivate *priv = self->priv;

	return gv_station_list_prev(priv->station_list, priv->station,
	                            priv->repeat, priv->shuffle);
}

GvStation *
gv_player_get_next_station(GvPlayer *self)
{
	GvPlayerPrivate *priv = self->priv;

	return gv_station_list_next(priv->station_list, priv->station,
	                            priv->repeat, priv->shuffle);
}

static const gchar *
gv_player_get_station_uri(GvPlayer *self)
{
	GvStation *station;
	const gchar *station_uri;

	station = gv_player_get_station(self);
	if (station == NULL)
		return NULL;

	station_uri = gv_station_get_uri(station);
	return station_uri;
}

void
gv_player_set_station(GvPlayer *self, GvStation *station)
{
	GvPlayerPrivate *priv = self->priv;

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

	gv_player_set_metadata(self, NULL);

	g_object_notify(G_OBJECT(self), "station");
	g_object_notify(G_OBJECT(self), "station-uri");

	INFO("Station set to '%s'", station ? gv_station_get_name_or_uri(station) : NULL);
}

gboolean
gv_player_set_station_by_name(GvPlayer *self, const gchar *name)
{
	GvPlayerPrivate *priv = self->priv;
	GvStation *station = NULL;

	station = gv_station_list_find_by_name(priv->station_list, name);
	if (station == NULL) {
		DEBUG("Station name '%s' not found in station list", name);
		return FALSE;
	}

	gv_player_set_station(self, station);
	return TRUE;
}

gboolean
gv_player_set_station_by_uri(GvPlayer *self, const gchar *uri)
{
	GvPlayerPrivate *priv = self->priv;
	GvStation *station = NULL;

	/* Look for the station in the station list */
	station = gv_station_list_find_by_uri(priv->station_list, uri);
	if (station == NULL) {
		DEBUG("Station uri '%s' not found in station list", uri);
		return FALSE;
	}

	gv_player_set_station(self, station);
	return TRUE;
}

gboolean
gv_player_set_station_by_guessing(GvPlayer *self, const gchar *string)
{
	GvPlayerPrivate *priv = self->priv;
	GvStation *station = NULL;

	station = gv_station_list_find_by_guessing(priv->station_list, string);
	if (station == NULL) {
		DEBUG("'%s' not found in station list", string);
		return FALSE;
	}

	gv_player_set_station(self, station);
	return TRUE;
}

const gchar *
gv_player_get_stream_uri(GvPlayer *self)
{
	GvPlayerPrivate *priv = self->priv;

	return gv_engine_get_stream_uri(priv->engine);
}

static void
gv_player_get_property(GObject    *object,
                       guint       property_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
	GvPlayer *self = GV_PLAYER(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_STATE:
		g_value_set_enum(value, gv_player_get_state(self));
		break;
	case PROP_VOLUME:
		g_value_set_uint(value, gv_player_get_volume(self));
		break;
	case PROP_MUTE:
		g_value_set_boolean(value, gv_player_get_mute(self));
		break;
	case PROP_REPEAT:
		g_value_set_boolean(value, gv_player_get_repeat(self));
		break;
	case PROP_SHUFFLE:
		g_value_set_boolean(value, gv_player_get_shuffle(self));
		break;
	case PROP_AUTOPLAY:
		g_value_set_boolean(value, gv_player_get_autoplay(self));
		break;
	case PROP_METADATA:
		g_value_set_object(value, gv_player_get_metadata(self));
		break;
	case PROP_STATION:
		g_value_set_object(value, gv_player_get_station(self));
		break;
	case PROP_STATION_URI:
		g_value_set_string(value, gv_player_get_station_uri(self));
		break;
	case PROP_PREV_STATION:
		g_value_set_object(value, gv_player_get_prev_station(self));
		break;
	case PROP_NEXT_STATION:
		g_value_set_object(value, gv_player_get_next_station(self));
		break;
	case PROP_STREAM_URI:
		g_value_set_string(value, gv_player_get_stream_uri(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
gv_player_set_property(GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
	GvPlayer *self = GV_PLAYER(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_ENGINE:
		gv_player_set_engine(self, g_value_get_object(value));
		break;
	case PROP_STATION_LIST:
		gv_player_set_station_list(self, g_value_get_object(value));
		break;
	case PROP_VOLUME:
		gv_player_set_volume(self, g_value_get_uint(value));
		break;
	case PROP_MUTE:
		gv_player_set_mute(self, g_value_get_boolean(value));
		break;
	case PROP_REPEAT:
		gv_player_set_repeat(self, g_value_get_boolean(value));
		break;
	case PROP_SHUFFLE:
		gv_player_set_shuffle(self, g_value_get_boolean(value));
		break;
	case PROP_AUTOPLAY:
		gv_player_set_autoplay(self, g_value_get_boolean(value));
		break;
	case PROP_METADATA:
		gv_player_set_metadata(self, g_value_get_object(value));
		break;
	case PROP_STATION:
		gv_player_set_station(self, g_value_get_object(value));
		break;
	case PROP_STATION_URI:
		gv_player_set_station_by_uri(self, g_value_get_string(value));
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
gv_player_stop(GvPlayer *self)
{
	GvPlayerPrivate *priv = self->priv;

	/* To remember what we're doing */
	priv->wish = GV_PLAYER_WISH_TO_STOP;

	/* Stop playing */
	gv_engine_stop(priv->engine);
}

void
gv_player_play(GvPlayer *self)
{
	GvPlayerPrivate *priv = self->priv;
	GvStation *station;
	GSList *uris;

	/* If no station is set yet, take the first from the station list */
	if (priv->station == NULL) {
		GvStation *first_station;

		first_station = gv_station_list_first(priv->station_list);
		gv_player_set_station(self, first_station);
	}

	/* If there's still no station, return */
	station = priv->station;
	if (station == NULL)
		return;

	/* To remember what we're doing */
	priv->wish = GV_PLAYER_WISH_TO_PLAY;

	/* Stop playing */
	gv_engine_stop(priv->engine);

	/* Get station data */
	uris = gv_station_get_stream_uris(station);

	/* If there's no uris, that probably means that the station uri
	 * points to a playlist, and we need to download it.
	 */
	if (uris == NULL) {
		/* Download the playlist that contains the stream uris */
		if (!gv_station_download_playlist(station))
			WARNING("Can't download playlist");

		/* Downloading a playlist is an asynchronous operation.
		 * We have nothing left to do here.
		 */
		return;
	} else {
		const gchar *first_uri;

		/* Play the first uri */
		first_uri = (gchar *) uris->data;
		gv_engine_play(priv->engine, first_uri);
	}
}

gboolean
gv_player_next(GvPlayer *self)
{
	GvPlayerPrivate *priv = self->priv;
	GvStation *station;

	station = gv_player_get_next_station(self);

	if (station == NULL)
		return FALSE;

	gv_player_set_station(self, station);

	if (priv->wish == GV_PLAYER_WISH_TO_PLAY)
		gv_player_play(self);

	return TRUE;
}

gboolean
gv_player_prev(GvPlayer *self)
{
	GvPlayerPrivate *priv = self->priv;
	GvStation *station;

	station = gv_player_get_prev_station(self);

	if (station == NULL)
		return FALSE;

	gv_player_set_station(self, station);

	if (priv->wish == GV_PLAYER_WISH_TO_PLAY)
		gv_player_play(self);

	return TRUE;
}

void
gv_player_toggle(GvPlayer *self)
{
	GvPlayerPrivate *priv = self->priv;

	switch (priv->wish) {
	case GV_PLAYER_WISH_TO_STOP:
		gv_player_play(self);
		break;
	case GV_PLAYER_WISH_TO_PLAY:
		gv_player_stop(self);
		break;
	default:
		ERROR("Invalid wish: %d", priv->wish);
		/* Program execution stops here */
		break;
	}
}

void
gv_player_go(GvPlayer *self, const gchar *string_to_play)
{
	GvPlayerPrivate *priv = self->priv;

	/* If we have no argument, we rely on 'autoplay' */
	if (string_to_play == NULL) {
		if (priv->autoplay) {
			INFO("Autoplay is enabled, let's play");
			gv_player_play(self);
		}
		return;
	}

	/* If we have an argument, it might be anything.
	 * At first, check if we find it in the station list.
	 */
	if (gv_player_set_station_by_guessing(self, string_to_play)) {
		INFO("'%s' found in station list, let's play", string_to_play);
		gv_player_play(self);
		return;
	}

	/* Otherwise, if it's a valid uri, try to play it */
	if (is_uri_scheme_supported(string_to_play)) {
		GvStation *station;

		station = gv_station_new(NULL, string_to_play);
		gv_player_set_station(self, station);
		g_object_unref(station);

		INFO("'%s' is a valid uri, let's play", string_to_play);
		gv_player_play(self);
		return;
	}

	/* That looks like an invalid string then */
	{
		gchar *str;

		str = g_strdup_printf("'%s' is neither a known station or a valid uri",
		                      string_to_play);

		INFO("%s", str);
		gv_errorable_emit_error(GV_ERRORABLE(self), str);

		g_free(str);
	}
}

GvPlayer *
gv_player_new(GvEngine *engine, GvStationList *station_list)
{
	return g_object_new(GV_TYPE_PLAYER,
	                    "engine", engine,
	                    "station-list", station_list,
	                    NULL);
}

/*
 * GObject methods
 */

static void
gv_player_finalize(GObject *object)
{
	GvPlayer *self = GV_PLAYER(object);
	GvPlayerPrivate *priv = self->priv;

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
	G_OBJECT_CHAINUP_FINALIZE(gv_player, object);
}

static void
gv_player_constructed(GObject *object)
{
	GvPlayer *self = GV_PLAYER(object);
	GvPlayerPrivate *priv = self->priv;

	TRACE("%p", object);

	/* Initialize properties */
	priv->volume   = DEFAULT_VOLUME;
	priv->mute     = DEFAULT_MUTE;
	priv->repeat   = DEFAULT_REPEAT;
	priv->shuffle  = DEFAULT_SHUFFLE;
	priv->autoplay = DEFAULT_AUTOPLAY;
	priv->station  = NULL;

	/* Bind settings */
	g_settings_bind(gv_core_settings, "volume",
	                self, "volume", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(gv_core_settings, "mute",
	                self, "mute", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(gv_core_settings, "repeat",
	                self, "repeat", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(gv_core_settings, "shuffle",
	                self, "shuffle", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(gv_core_settings, "autoplay",
	                self, "autoplay", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(gv_core_settings, "station-uri",
	                self, "station-uri", G_SETTINGS_BIND_DEFAULT);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(gv_player, object);
}

static void
gv_player_init(GvPlayer *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = gv_player_get_instance_private(self);
}

static void
gv_player_class_init(GvPlayerClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = gv_player_finalize;
	object_class->constructed = gv_player_constructed;

	/* Properties */
	object_class->get_property = gv_player_get_property;
	object_class->set_property = gv_player_set_property;

	properties[PROP_ENGINE] =
	        g_param_spec_object("engine", "Engine", NULL,
	                            GV_TYPE_ENGINE,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_WRITABLE |
	                            G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_STATION_LIST] =
	        g_param_spec_object("station-list", "Station list", NULL,
	                            GV_TYPE_STATION_LIST,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_WRITABLE |
	                            G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_STATE] =
	        g_param_spec_enum("state", "Playback State", NULL,
	                          GV_PLAYER_STATE_ENUM_TYPE,
	                          GV_PLAYER_STATE_STOPPED,
	                          GV_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	properties[PROP_VOLUME] =
	        g_param_spec_uint("volume", "Volume In Percent", NULL,
	                          0, 100, DEFAULT_VOLUME,
	                          GV_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                          G_PARAM_READWRITE);

	properties[PROP_MUTE] =
	        g_param_spec_boolean("mute", "Mute", NULL,
	                             DEFAULT_MUTE,
	                             GV_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                             G_PARAM_READWRITE);

	properties[PROP_REPEAT] =
	        g_param_spec_boolean("repeat", "Repeat", NULL,
	                             DEFAULT_REPEAT,
	                             GV_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                             G_PARAM_READWRITE);

	properties[PROP_SHUFFLE] =
	        g_param_spec_boolean("shuffle", "Shuffle", NULL,
	                             DEFAULT_SHUFFLE,
	                             GV_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                             G_PARAM_READWRITE);

	properties[PROP_AUTOPLAY] =
	        g_param_spec_boolean("autoplay", "Autoplay On Startup", NULL,
	                             DEFAULT_AUTOPLAY,
	                             GV_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                             G_PARAM_READWRITE);

	properties[PROP_METADATA] =
	        g_param_spec_object("metadata", "Current Metadata", NULL,
	                            GV_TYPE_METADATA,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_STATION] =
	        g_param_spec_object("station", "Current Station", NULL,
	                            GV_TYPE_STATION,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_STATION_URI] =
	        g_param_spec_string("station-uri", "Current Station Uri",
	                            "This is a workaround for serialization, because we "
	                            "can't serialize the 'station' property (GObject type)",
	                            NULL,
	                            GV_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                            G_PARAM_READWRITE);

	properties[PROP_PREV_STATION] =
	        g_param_spec_object("prev", "Previous Station", NULL,
	                            GV_TYPE_STATION,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	properties[PROP_NEXT_STATION] =
	        g_param_spec_object("next", "Next Station", NULL,
	                            GV_TYPE_STATION,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	properties[PROP_STREAM_URI] =
	        g_param_spec_string("stream-uri", "Curent Stream Uri", NULL, NULL,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	g_object_class_install_properties(object_class, PROP_N, properties);
}
