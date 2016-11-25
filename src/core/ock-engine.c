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

#include <glib.h>
#include <glib-object.h>
#include <gst/gst.h>
#include <gst/audio/streamvolume.h>

#include "additions/glib-object.h"
#include "additions/gst.h"

#include "framework/log.h"
#include "framework/ock-param-specs.h"

#include "core/ock-engine.h"
#include "core/ock-core-enum-types.h"
#include "core/ock-metadata.h"

/* Uncomment to dump incoming tags on the GstBus */

#define DUMP_TAGS

/*
 * Signals
 */

enum {
	SIGNAL_ERROR,
	/* Number of signals */
	SIGNAL_N
};

static guint signals[SIGNAL_N];

/*
 * Properties
 */

#define DEFAULT_VOLUME 1.0

enum {
	/* Reserved */
	PROP_0,
	/* Properties - refer to class_init() for more details */
	PROP_STATE,
	PROP_VOLUME,
	PROP_MUTE,
	PROP_STREAM_URI,
	PROP_METADATA,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * GObject definitions
 */

struct _OckEnginePrivate {
	/* GStreamer stuff */
	GstElement     *playbin;
	GstBus         *bus;
	/* Properties */
	OckEngineState  state;
	gdouble         volume;
	gboolean        mute;
	gchar          *stream_uri;
	OckMetadata    *metadata;
};

typedef struct _OckEnginePrivate OckEnginePrivate;

struct _OckEngine {
	/* Parent instance structure */
	GObject           parent_instance;
	/* Private data */
	OckEnginePrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(OckEngine, ock_engine, G_TYPE_OBJECT)

/*
 * GStreamer helpers
 */

static void
set_gst_state(GstElement *playbin, GstState new)
{
	GstStateChangeReturn ret;

	TRACE("%p, %s", playbin, gst_element_state_get_name(new));

	ret = gst_element_set_state(playbin, new);

	switch (ret) {
	case GST_STATE_CHANGE_SUCCESS:
		//DEBUG("State change success");
		break;
	case GST_STATE_CHANGE_ASYNC:
		DEBUG("State will change async");
		break;
	case GST_STATE_CHANGE_FAILURE:
		DEBUG("State change failure");
		break;
	case GST_STATE_CHANGE_NO_PREROLL:
		DEBUG("State change no_preroll");
		break;
	default:
		WARNING("Unhandled state: %d", ret);
		break;
	}
}

#if 0
static GstState
get_gst_state(GstElement *playbin)
{
	GstStateChangeReturn ret;
	GstState state, pending;

	/* When using that for real, we should set a timeout,
	 * since it's possible that the call hangs...
	 */
	ret = gst_element_get_state(playbin, &state, &pending, GST_CLOCK_TIME_NONE);

	switch (ret) {
	case GST_STATE_CHANGE_SUCCESS:
		//DEBUG("State change succeeded");
		break;
	case GST_STATE_CHANGE_ASYNC:
		DEBUG("State will change async");
		break;
	case GST_STATE_CHANGE_FAILURE:
		DEBUG("State change failed");
		break;
	case GST_STATE_CHANGE_NO_PREROLL:
		DEBUG("State change no_preroll");
		break;
	default:
		WARNING("Unhandled state: %d", ret);
		break;
	}

	return state;
}
#endif

static OckMetadata *
taglist_to_metadata(GstTagList *taglist)
{
	OckMetadata *metadata;
	GDate *date = NULL;
	gchar *artist = NULL;
	gchar *title = NULL;
	gchar *album = NULL;
	gchar *genre = NULL;
	gchar *year = NULL;
	gchar *comment = NULL;
	guint  bitrate = 0;

	/* Get info from taglist */
	gst_tag_list_get_string_index(taglist, GST_TAG_ARTIST, 0, &artist);
	gst_tag_list_get_string_index(taglist, GST_TAG_TITLE, 0, &title);
	gst_tag_list_get_string_index(taglist, GST_TAG_ALBUM, 0, &album);
	gst_tag_list_get_string_index(taglist, GST_TAG_GENRE, 0, &genre);
	gst_tag_list_get_date_index  (taglist, GST_TAG_DATE, 0, &date);
	gst_tag_list_get_string_index(taglist, GST_TAG_COMMENT, 0, &comment);
	gst_tag_list_get_uint_index  (taglist, GST_TAG_BITRATE, 0, &bitrate);
	if (date && g_date_valid(date))
		year = g_strdup_printf("%d", g_date_get_year(date));

	/* Create new metadata object */
	metadata = g_object_new(OCK_TYPE_METADATA,
	                        "artist", artist,
	                        "title", title,
	                        "album", album,
	                        "genre", genre,
	                        "year", year,
	                        "comment", comment,
	                        "bitrate", &bitrate,
	                        NULL);

	/* Freedom for the braves */
	g_free(artist);
	g_free(title);
	g_free(album);
	g_free(genre);
	g_free(year);
	g_free(comment);
	if (date)
		g_date_free(date);

	return metadata;
}

/*
 * Signal handlers
 */

static void
on_playbin_volume(GObject    *object G_GNUC_UNUSED,
                  GParamSpec *pspec G_GNUC_UNUSED,
                  OckEngine  *self)
{
	OckEnginePrivate *priv = self->priv;
	gdouble volume;

	volume = gst_stream_volume_get_volume(
	                 GST_STREAM_VOLUME(priv->playbin),
	                 GST_STREAM_VOLUME_FORMAT_CUBIC);

	/* We receive this signal each time the playback is started,
	 * in such case it's safe to ignore it.
	 */
	if (priv->volume == volume)
		return;

	priv->volume = volume;
	g_object_notify(G_OBJECT(self), "volume");
}

static void
on_playbin_mute(GObject    *object G_GNUC_UNUSED,
                GParamSpec *pspec G_GNUC_UNUSED,
                OckEngine  *self)
{
	OckEnginePrivate *priv = self->priv;
	gboolean mute;

	mute = gst_stream_volume_get_mute(GST_STREAM_VOLUME(priv->playbin));

	/* We receive this signal each time the playback is started,
	 * in such case it's safe to ignore it.
	 */
	if (priv->mute == mute)
		return;

	priv->mute = mute;
	g_object_notify(G_OBJECT(self), "mute");
}

static void
on_playbin_uri(GObject    *object G_GNUC_UNUSED,
               GParamSpec *pspec G_GNUC_UNUSED,
               OckEngine  *self)
{
	OckEnginePrivate *priv = self->priv;
	gchar *uri;

	g_object_get(priv->playbin, "uri", &uri, NULL);

	/* The value should match the one we have internally */
	if (g_strcmp0(uri, priv->stream_uri) != 0)
		WARNING("Playbin notify: received unexpected uri '%s'", uri);

	g_object_notify(G_OBJECT(self), "stream-uri");
}

/*
 * Property accessors
 */

OckEngineState
ock_engine_get_state(OckEngine *self)
{
	return self->priv->state;
}

static void
ock_engine_set_state(OckEngine *self, OckEngineState state)
{
	OckEnginePrivate *priv = self->priv;

	if (priv->state == state)
		return;

	/* When playing, ignore the 'buffering' state */
	if (priv->state == OCK_ENGINE_STATE_PLAYING)
		if (state == OCK_ENGINE_STATE_BUFFERING)
			return;

	/* When buffering, ignore parasite states */
	//TODO Improve that in future. For example, if connection is broken during buffering,
	//     I think it will break...
	if (priv->state == OCK_ENGINE_STATE_BUFFERING)
		if (state == OCK_ENGINE_STATE_STOPPED)
			return;

	priv->state = state;

	g_object_notify(G_OBJECT(self), "state");
}

gdouble
ock_engine_get_volume(OckEngine *self)
{
	return self->priv->volume;
}

void
ock_engine_set_volume(OckEngine *self, gdouble volume)
{
	OckEnginePrivate *priv = self->priv;

	volume = volume > 1.0 ? 1.0 :
	         volume < 0.0 ? 0.0 : volume;

	if (priv->volume == volume)
		return;

	gst_stream_volume_set_volume(GST_STREAM_VOLUME(priv->playbin),
	                             GST_STREAM_VOLUME_FORMAT_CUBIC, volume);

	/* priv->volume will be updated in a callback */
}

gboolean
ock_engine_get_mute(OckEngine *self)
{
	return self->priv->mute;
}

void
ock_engine_set_mute(OckEngine *self, gboolean mute)
{
	OckEnginePrivate *priv = self->priv;

	if (priv->mute == mute)
		return;

	gst_stream_volume_set_mute(GST_STREAM_VOLUME(priv->playbin), mute);

	/* priv->mute will be updated in a callback */
}

const gchar *
ock_engine_get_stream_uri(OckEngine *self)
{
	return self->priv->stream_uri;
}

void
ock_engine_set_stream_uri(OckEngine *self, const gchar *uri)
{
	OckEnginePrivate *priv = self->priv;

	/* Do nothing in case this stream is already set */
	if (!g_strcmp0(priv->stream_uri, uri))
		return;

	/* Clear previous data */
	g_free(priv->stream_uri);
	priv->stream_uri = NULL;

	if (priv->metadata) {
		g_object_unref(priv->metadata);
		priv->metadata = NULL;
	}

	/* Assign new data */
	priv->stream_uri = g_strdup(uri);

	/* Stop playback if playing */
	if (ock_engine_get_state(self) == OCK_ENGINE_STATE_PLAYING)
		ock_engine_stop(self);

	/* Set URI to playbin */
	g_object_set(priv->playbin, "uri", uri, NULL);

	/* Care for a little bit of debug ? */
	DEBUG("Internal stream uri set to '%s'", uri);
}

OckMetadata *
ock_engine_get_metadata(OckEngine *self)
{
	return self->priv->metadata;
}

static void
ock_engine_get_property(GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
	OckEngine *self = OCK_ENGINE(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_STATE:
		g_value_set_enum(value, ock_engine_get_state(self));
		break;
	case PROP_VOLUME:
		g_value_set_double(value, ock_engine_get_volume(self));
		break;
	case PROP_MUTE:
		g_value_set_boolean(value, ock_engine_get_mute(self));
		break;
	case PROP_STREAM_URI:
		g_value_set_string(value, ock_engine_get_stream_uri(self));
		break;
	case PROP_METADATA:
		g_value_set_object(value, ock_engine_get_metadata(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
ock_engine_set_property(GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
	OckEngine *self = OCK_ENGINE(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_VOLUME:
		ock_engine_set_volume(self, g_value_get_double(value));
		break;
	case PROP_MUTE:
		ock_engine_set_mute(self, g_value_get_boolean(value));
		break;
	case PROP_STREAM_URI:
		ock_engine_set_stream_uri(self, g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Private stuff
 */

static void
ock_engine_start_buffering(OckEngine *self)
{
	OckEnginePrivate *priv = self->priv;

	/* Set to PAUSE, so that the playbin starts buffering data.
	 * Playback will start as soon as buffering is finished.
	 * Internal state shouldn't be touched, it will be updated
	 * as soon as buffering is started.
	 */
	set_gst_state(priv->playbin, GST_STATE_PAUSED);
}

static void
ock_engine_start_playing(OckEngine *self)
{
	OckEnginePrivate *priv = self->priv;

	set_gst_state(priv->playbin, GST_STATE_PLAYING);
}


static void
ock_engine_stop_playback(OckEngine *self)
{
	OckEnginePrivate *priv = self->priv;

	/* Radical way to stop: set state to NULL.
	 * However, if we do that, we don't receive any message
	 * anymore from gst bus, so we can't rely on that to update
	 * our internal state. We must set it manually.
	 */
	set_gst_state(priv->playbin, GST_STATE_NULL);
	ock_engine_set_state(self, OCK_ENGINE_STATE_STOPPED);
}

/*
 * Public methods
 */

void
ock_engine_play(OckEngine *self)
{
	OckEnginePrivate *priv = self->priv;

	if (priv->stream_uri == NULL) {
		g_signal_emit(self, signals[SIGNAL_ERROR], 0,
		              OCK_ENGINE_ERROR_STREAM_UNDEFINED);
		return;
	}

	ock_engine_start_buffering(self);
}

void
ock_engine_stop(OckEngine *self)
{
	ock_engine_stop_playback(self);
}

OckEngine *
ock_engine_new(void)
{
	return g_object_new(OCK_TYPE_ENGINE, NULL);
}

/*
 * Gst bus signal handlers
 */

static gboolean
on_bus_eos(GstBus *bus G_GNUC_UNUSED, GstMessage *msg G_GNUC_UNUSED, OckEngine *self)
{
	DEBUG("End of stream");

	// TODO Send signal ?
	ock_engine_stop_playback(self);

	return TRUE;
}

static gboolean
on_bus_error(GstBus *bus G_GNUC_UNUSED, GstMessage *msg, OckEngine *self)
{
	GError *error;
	gchar  *debug;

	gst_message_parse_error(msg, &error, &debug);

	/* Display error */
	DEBUG("Gst bus error msg %d:%d: %s", error->domain, error->code, error->message);
	DEBUG("Gst bus error debug : %s", debug);

	/* Handle errors */
	if (g_error_matches(error, GST_RESOURCE_ERROR, GST_RESOURCE_ERROR_NOT_FOUND)) {
		if (g_str_has_prefix(error->message, "Could not resolve server name")) {
			g_signal_emit(self, signals[SIGNAL_ERROR], 0,
			              OCK_ENGINE_ERROR_SERVER_NAME_UNRESOLVED);
		} else if (g_str_has_prefix(error->message, "Not Available")) {
			g_signal_emit(self, signals[SIGNAL_ERROR], 0,
			              OCK_ENGINE_ERROR_STREAM_UNAVAILABLE);
		}
	} else if (g_error_matches(error, GST_CORE_ERROR, GST_CORE_ERROR_MISSING_PLUGIN)) {
		/* This may happen with public wifi, where the router blocks WAN access
		 * and requires you to enter a code, or agree terms of use, or this
		 * kind of things.
		 * In this case, every request returns a http page.
		 */
		g_signal_emit(self, signals[SIGNAL_ERROR], 0,
		              OCK_ENGINE_ERROR_STREAM_FORMAT_UNRECOGNIZED);
	}

	g_error_free(error);
	g_free(debug);

	/* Stop playback */
	ock_engine_stop_playback(self);

	return TRUE;
}

static gboolean
on_bus_warning(GstBus *bus G_GNUC_UNUSED, GstMessage *msg, OckEngine *self G_GNUC_UNUSED)
{
	GError *warning;
	gchar  *debug;

	gst_message_parse_warning(msg, &warning, &debug);

	/* Display warning */
	WARNING("Gst bus warning msg %d:%d: %s", warning->domain, warning->code, warning->message);
	WARNING("Gst bus warning debug : %s", debug);

	g_error_free(warning);
	g_free(debug);

	return TRUE;
}

static gboolean
on_bus_info(GstBus *bus G_GNUC_UNUSED, GstMessage *msg, OckEngine *self G_GNUC_UNUSED)
{
	GError *info;
	gchar  *debug;

	gst_message_parse_info(msg, &info, &debug);

	INFO("Gst bus info msg %d:%d: %s", info->domain, info->code, info->message);
	INFO("Gst bus info debug : %s", debug);

	g_error_free(info);
	g_free(debug);

	return TRUE;
}

#ifdef DUMP_TAGS
static void
tag_list_foreach_dump(const GstTagList *list, const gchar *tag,
                      gpointer data G_GNUC_UNUSED)
{
	gchar *str = NULL;

	switch (gst_tag_get_type(tag)) {
	case G_TYPE_STRING:
		gst_tag_list_get_string_index(list, tag, 0, &str);
		break;
	case G_TYPE_INT: {
		gint val;
		gst_tag_list_get_int_index(list, tag, 0, &val);
		str = g_strdup_printf("%d", val);
		break;
	}
	case G_TYPE_UINT: {
		guint val;
		gst_tag_list_get_uint_index(list, tag, 0, &val);
		str = g_strdup_printf("%u", val);
		break;
	}
	case G_TYPE_DOUBLE: {
		gdouble val;
		gst_tag_list_get_double_index(list, tag, 0, &val);
		str = g_strdup_printf("%lg", val);
		break;
	}
	case G_TYPE_BOOLEAN: {
		gboolean val;
		gst_tag_list_get_boolean_index(list, tag, 0, &val);
		str = g_strdup_printf("%s", val ? "true" : "false");
		break;
	}
	}

	DEBUG("%10s '%20s' (%d elem): %s",
	      g_type_name(gst_tag_get_type(tag)),
	      tag,
	      gst_tag_list_get_tag_size(list, tag),
	      str);
	g_free(str);

}
#endif /* DUMP_TAGS */

static gboolean
on_bus_tag(GstBus *bus G_GNUC_UNUSED, GstMessage *msg, OckEngine *self)
{
	OckEnginePrivate *priv = self->priv;
	GstTagList *taglist = NULL;
	OckMetadata *metadata;

	TRACE("... %s, %p", GST_OBJECT_NAME(msg->src), self);

	/* Parse tag list */
	gst_message_parse_tag(msg, &taglist);

	/* Dumping may be needed to debug */
#ifdef DUMP_TAGS
	DEBUG("-- Dumping taglist...");
	gst_tag_list_foreach(taglist, (GstTagForeachFunc) tag_list_foreach_dump, NULL);
	DEBUG("-- Done --");
#endif /* DUMP_TAGS */

	/* Turn into metadata */
	metadata = taglist_to_metadata(taglist);

	/* Finished with taglist already */
	gst_tag_list_unref(taglist);

	/* Metadata can be quite noisy, so let's cut it short.
	 * From my experience, 'title' is the most important field,
	 * and it's likely that it's the only one filled, containing
	 * everything (title, artist and more).
	 * So, we require this field to be filled. If it's not, this
	 * metadata is considered as noise, and discarded.
	 */
	if (ock_metadata_get_title(metadata) == NULL) {
		g_object_unref(metadata);
		return TRUE;
	}

	/* Compare that with what we already have */
	if (priv->metadata && ock_metadata_is_equal(priv->metadata, metadata)) {
		DEBUG("Metadata identical, ignoring...");
		g_object_unref(metadata);
		return TRUE;
	}

	/* Save this new metadata */
	if (priv->metadata)
		g_object_unref(priv->metadata);

	priv->metadata = metadata;

	/* Notify */
	g_object_notify(G_OBJECT(self), "metadata");

	return TRUE;
}

static gboolean
on_bus_buffering(GstBus *bus G_GNUC_UNUSED, GstMessage *msg, OckEngine *self)
{
	static gint prev_percent = 0;
	gint percent = 0;

	/* Set current engine state to buffering */
	ock_engine_set_state(self, OCK_ENGINE_STATE_BUFFERING);

	/* Parse message */
	gst_message_parse_buffering(msg, &percent);

	/* Display buffering steps 20 by 20 */
	if (ABS(percent - prev_percent) > 20) {
		prev_percent = percent;
		DEBUG("Buffering (%3u %%)", percent);
	}

	/* When buffering is complete, start playing */
	if (percent >= 100) {
		DEBUG("Buffering complete");
		ock_engine_start_playing(self);
	}

	return TRUE;
}

static gboolean
on_bus_state_changed(GstBus *bus G_GNUC_UNUSED, GstMessage *msg, OckEngine *self)
{
	GstState old, new, pending;
	OckEngineState engine_state;

	gst_message_parse_state_changed(msg, &old, &new, &pending);

	DEBUG("old: %s, new: %s, pending: %s",
	      gst_element_state_get_name(old),
	      gst_element_state_get_name(new),
	      gst_element_state_get_name(pending));

	switch (new) {
	case GST_STATE_PLAYING:
		engine_state = OCK_ENGINE_STATE_PLAYING;
		break;
	case GST_STATE_VOID_PENDING:
	case GST_STATE_NULL:
	case GST_STATE_READY:
	case GST_STATE_PAUSED:
		engine_state = OCK_ENGINE_STATE_STOPPED;
		break;
	default:
		WARNING("Unhandled GstState: %d", new);
		engine_state = OCK_ENGINE_STATE_STOPPED;
		break;
	}

	ock_engine_set_state(self, engine_state);

	return TRUE;
}

/*
 * GObject methods
 */

static void
ock_engine_finalize(GObject *object)
{
	OckEngine *self = OCK_ENGINE(object);
	OckEnginePrivate *priv = self->priv;

	TRACE("%p", object);

	/* Stop GStreamer at first */
	ock_engine_stop_playback(self);

	/* Unref metadata */
	if (priv->metadata)
		g_object_unref(priv->metadata);

	/* Free stream uri */
	g_free(priv->stream_uri);

	/* Remove the bus signal watch */
	gst_bus_remove_signal_watch(priv->bus);

	/* Unref the bus */
	g_signal_handlers_disconnect_by_data(priv->bus, self);
	g_object_unref(priv->bus);

	/* Unref the playbin */
	g_signal_handlers_disconnect_by_data(priv->playbin, self);
	g_object_unref(priv->playbin);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_engine, object);
}

static void
ock_engine_constructed(GObject *object)
{
	OckEngine *self = OCK_ENGINE(object);
	OckEnginePrivate *priv = self->priv;
	GstElement *playbin;
	GstElement *fakesink;
	GstBus *bus;

	/* Initialize properties */
	priv->volume = DEFAULT_VOLUME;

	/* Gstreamer must be initialized, let's check that */
	g_assert(gst_is_initialized());

	/* Make the playbin */
	playbin = gst_element_factory_make("playbin", "playbin");
	g_assert(playbin != NULL);
	priv->playbin = g_object_ref_sink(playbin);
	// factory_make() returns floating ref, let's sink

	/* Disable video */
	fakesink = gst_element_factory_make("fakesink", "fakesink");
	g_assert(fakesink != NULL);
	g_object_set(playbin, "video-sink", fakesink, NULL);
	// factory_make() returns floating ref, no need to unref

	/* Connect to signals from playbin */
	g_signal_connect(playbin, "notify::volume", G_CALLBACK(on_playbin_volume), self);
	g_signal_connect(playbin, "notify::mute", G_CALLBACK(on_playbin_mute), self);
	g_signal_connect(playbin, "notify::uri", G_CALLBACK(on_playbin_uri), self);

	/* Get a reference to the message bus */
	bus = gst_element_get_bus(playbin);
	g_assert(bus != NULL);
	priv->bus = bus;
	// get_bus() returns full ref, no need to ref again

	/* Add a bus signal watch */
	gst_bus_add_signal_watch(bus);

	/* Connect to signals from the bus */
	g_signal_connect(bus, "message::eos", G_CALLBACK(on_bus_eos), self);
	g_signal_connect(bus, "message::error", G_CALLBACK(on_bus_error), self);
	g_signal_connect(bus, "message::warning", G_CALLBACK(on_bus_warning), self);
	g_signal_connect(bus, "message::info", G_CALLBACK(on_bus_info), self);
	g_signal_connect(bus, "message::tag", G_CALLBACK(on_bus_tag), self);
	g_signal_connect(bus, "message::buffering", G_CALLBACK(on_bus_buffering), self);
	g_signal_connect(bus, "message::state-changed", G_CALLBACK(on_bus_state_changed), self);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_engine, object);
}

static void
ock_engine_init(OckEngine *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_engine_get_instance_private(self);
}

static void
ock_engine_class_init(OckEngineClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_engine_finalize;
	object_class->constructed = ock_engine_constructed;

	/* Properties */
	object_class->get_property = ock_engine_get_property;
	object_class->set_property = ock_engine_set_property;

	properties[PROP_STATE] =
	        g_param_spec_enum("state", "Playback state", NULL,
	                          OCK_ENGINE_STATE_ENUM_TYPE,
	                          OCK_ENGINE_STATE_STOPPED,
	                          OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	properties[PROP_VOLUME] =
	        g_param_spec_double("volume", "Volume", NULL,
	                            0.0, 1.0, DEFAULT_VOLUME,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_MUTE] =
	        g_param_spec_boolean("mute", "Mute", NULL,
	                             FALSE,
	                             OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_STREAM_URI] =
	        g_param_spec_string("stream-uri", "Stream uri", NULL, NULL,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_METADATA] =
	        g_param_spec_object("metadata", "Current metadata", NULL,
	                            OCK_TYPE_METADATA,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	g_object_class_install_properties(object_class, PROP_N, properties);

	/* Signals */
	signals[SIGNAL_ERROR] =
	        g_signal_new("error", G_TYPE_FROM_CLASS(class),
	                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                     0, NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE,
	                     1, OCK_ENGINE_ERROR_ENUM_TYPE);
}
