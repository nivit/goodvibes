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

#include "additions/glib-object.h"

#include "libgszn/gszn.h"

#include "framework/log.h"
#include "framework/uri-schemes.h"
#include "framework/ock-framework.h"

#include "core/ock-station-list.h"

// WISHED Try with a huge number of stations to see how it behaves.
//        It might be slow. The implementation never tried to be fast.
// TODO   More test (load/save) with empty list.

/*
 * FIP <http://www.fipradio.fr/>
 * Just the best radios you'll ever listen to.
 */

#define FIP_STATIONS      \
	"<Station>" \
	"  <name>FIP Paris</name>" \
	"  <uri>http://direct.fipradio.fr/live/fip-midfi.mp3</uri>" \
	"</Station>" \
	"<Station>" \
	"  <name>FIP Autour du Rock</name>" \
	"  <uri>http://direct.fipradio.fr/live/fip-webradio1.mp3</uri>" \
	"</Station>" \
	"<Station>" \
	"  <name>FIP Autour du Jazz</name>" \
	"  <uri>http://direct.fipradio.fr/live/fip-webradio2.mp3</uri>" \
	"</Station>" \
	"<Station>" \
	"  <name>FIP Autour du Groove</name>" \
	"  <uri>http://direct.fipradio.fr/live/fip-webradio3.mp3</uri>" \
	"</Station>" \
	"<Station>" \
	"  <name>FIP Autour du Monde</name>" \
	"  <uri>http://direct.fipradio.fr/live/fip-webradio4.mp3</uri>" \
	"</Station>" \
	"<Station>" \
	"  <name>FIP Tout nouveau, tout FIP</name>" \
	"  <uri>http://direct.fipradio.fr/live/fip-webradio5.mp3</uri>" \
	"</Station>" \
	"<Station>" \
	"  <name>FIP Evenementielle</name>" \
	"  <uri>http://direct.fipradio.fr/live/fip-webradio6.mp3</uri>" \
	"</Station>"

/*
 * More of my favorite french radios.
 * - Nova       <http://www.novaplanet.com/>
 * - Grenouille <http://www.radiogrenouille.com/>
 */

#define FRENCH_STATIONS   \
	"<Station>" \
	"  <name>Nova</name>" \
	"  <uri>http://broadcast.infomaniak.net/radionova-high.mp3</uri>" \
	"</Station>" \
	"<Station>" \
	"  <name>Radio Grenouille</name>" \
	"  <uri>http://live.radiogrenouille.com/live</uri>" \
	"</Station>"

/*
 * Broken stations.
 */

#define TESTING_BROKEN_STATIONS   \
	"<Station>" \
	"  <name>Broken - FIP old url</name>" \
	"  <uri>http://audio.scdn.arkena.com/11016/fip-midfi128.mp3</uri>" \
	"</Station>"

/*
 * Various playlist formats.
 * - Swiss Internet Radio (Public Domain Radio) <http://www.swissradio.ch/>
 */

#define TESTING_PLAYLIST_STATIONS         \
	"<Station>" \
	"  <name>M3U - Swiss Internet Radio Classical</name>" \
	"  <uri>http://www.swissradio.ch/streams/6034.m3u</uri>" \
	"</Station>" \
	"<Station>" \
	"  <name>ASX - Swiss Internet Radio Classical</name>" \
	"  <uri>http://www.swissradio.ch/streams/6034.asx</uri>" \
	"</Station>" \
	"<Station>" \
	"  <name>RAM - Swiss Internet Radio Classical</name>" \
	"  <uri>http://www.swissradio.ch/streams/6034.ram</uri>" \
	"</Station>"

/*
 * More radios, for testing.
 */

#define TESTING_MORE_STATIONS     \
	"<Station>" \
	"  <uri>http://www.netradio.fr:8000/A0RemzouilleRadio.xspf</uri>" \
	"</Station>" \
	"<Station>" \
	"  <uri>http://player.100p.nl/livestream.asx</uri>" \
	"</Station>" \
	"<Station>" \
	"  <uri>http://vt-net.org/WebRadio/live/8056.m3u</uri>" \
	"</Station>" \
	"<Station>" \
	"  <uri>'http://www.neradio.se/listen.pls</uri>" \
	"</Station>"

/*
 * Default station list, loaded if no station list file is found
 */

#define DEFAULT_STATIONS_DEV      \
	FIP_STATIONS \
	FRENCH_STATIONS \
	TESTING_BROKEN_STATIONS \
	TESTING_PLAYLIST_STATIONS \
	TESTING_MORE_STATIONS \
	 
#define DEFAULT_STATIONS_PROD     \
	FIP_STATIONS \
	FRENCH_STATIONS

#define DEFAULT_STATION_LIST      \
	"<OvercookedStationList>" \
	DEFAULT_STATIONS_PROD \
	"</OvercookedStationList>"

/*
 * Save delay - how long do we wait to write configuration to file
 */

#define SAVE_DELAY 1

/*
 * Signals
 */

enum {
	SIGNAL_STATION_ADDED,
	SIGNAL_STATION_REMOVED,
	SIGNAL_STATION_MODIFIED,
	SIGNAL_STATION_MOVED,
	/* Number of signals */
	SIGNAL_N
};

static guint signals[SIGNAL_N];

/*
 * GObject definitions
 */

struct _OckStationListPrivate {
	/* Load/save pathes */
	GSList *load_pathes;
	gchar  *save_path;
	/* Timeout id, > 0 if a save operation is scheduled */
	guint   save_timeout_id;
	/* Serialization objects */
	GsznSettings *serialization_settings;
	/* Ordered list of stations */
	GList  *stations;
	/* Shuffled list of stations, automatically created
	 * and destroyed when needed.
	 */
	GList  *shuffled;
};

typedef struct _OckStationListPrivate OckStationListPrivate;

struct _OckStationList {
	/* Parent instance structure */
	GObject                parent_instance;
	/* Private data */
	OckStationListPrivate *priv;
};

G_DEFINE_TYPE_WITH_CODE(OckStationList, ock_station_list, G_TYPE_OBJECT,
                        G_ADD_PRIVATE(OckStationList)
                        G_IMPLEMENT_INTERFACE(OCK_TYPE_ERRORABLE,
                                        ock_errorable_dummy_interface_init))

/*
 * Serialization settings
 */

static gchar *
serialize_object_name(const gchar *object_type_name)
{
	if (g_str_has_prefix(object_type_name, "Ock"))
		object_type_name += 3;

	return g_strdup(object_type_name);
}

static gchar *
deserialize_object_name(const gchar *object_name)
{
	return g_strconcat("Ock", object_name, NULL);
}

/*
 * Iterator implementation
 */

struct _OckStationListIter {
	GList *head, *item;
};

OckStationListIter *
ock_station_list_iter_new(OckStationList *self)
{
	GList *list = self->priv->stations;
	OckStationListIter *iter;

	iter = g_new0(OckStationListIter, 1);
	iter->head = g_list_copy_deep(list, (GCopyFunc) g_object_ref, NULL);
	iter->item = iter->head;

	return iter;
}

void
ock_station_list_iter_free(OckStationListIter *iter)
{
	g_assert(iter != NULL);

	g_list_free_full(iter->head, g_object_unref);
	g_free(iter);
}

gboolean
ock_station_list_iter_loop(OckStationListIter *iter, OckStation **station)
{
	g_assert(iter != NULL);
	g_assert(station != NULL);

	*station = NULL;

	if (iter->item == NULL)
		return FALSE;

	*station = iter->item->data;
	iter->item = iter->item->next;

	return TRUE;
}

/*
 * GList additions
 */

static gint
glist_sortfunc_random(gconstpointer a G_GNUC_UNUSED, gconstpointer b G_GNUC_UNUSED)
{
	return g_random_boolean() ? 1 : -1;
}

static GList *
g_list_shuffle(GList *list)
{
	/* Shuffle twice. Credits goes to:
	 * http://www.linuxforums.org/forum/programming-scripting/
	 * 202125-fastest-way-shuffle-linked-list.html
	 */
	list = g_list_sort(list, glist_sortfunc_random);
	list = g_list_sort(list, glist_sortfunc_random);
	return list;
}

static GList *
g_list_copy_deep_shuffle(GList *list, GCopyFunc func, gpointer user_data)
{
	list = g_list_copy_deep(list, func, user_data);
	list = g_list_shuffle(list);
	return list;
}

/*
 * Helpers
 */

static gint
are_stations_similar(OckStation *s1, OckStation *s2)
{
	if (s1 == s2) {
		WARNING("Stations %p and %p are the same", s1, s2);
		return 0;
	}

	if (s1 == NULL || s2 == NULL)
		return -1;

	/* Compare uids */
	const gchar *s1_uid, *s2_uid;
	s1_uid = ock_station_get_uid(s1);
	s2_uid = ock_station_get_uid(s2);

	if (!g_strcmp0(s1_uid, s2_uid)) {
		WARNING("Stations %p and %p have the same uid '%s'", s1, s2, s1_uid);
		return 0;
	}

	/* Compare names.
	 * Two stations who don't have name are different.
	 */
	const gchar *s1_name, *s2_name;
	s1_name = ock_station_get_name(s1);
	s2_name = ock_station_get_name(s2);

	if (s1_name == NULL && s2_name == NULL)
		return -1;

	if (!g_strcmp0(s1_name, s2_name)) {
		DEBUG("Stations %p and %p have the same name '%s'", s1, s2, s1_name);
		return 0;
	}

	/* Compare uris */
	const gchar *s1_uri, *s2_uri;
	s1_uri = ock_station_get_uri(s1);
	s2_uri = ock_station_get_uri(s2);

	if (!g_strcmp0(s1_uri, s2_uri)) {
		DEBUG("Stations %p and %p have the same uri '%s'", s1, s2, s1_uri);
		return 0;
	}

	return -1;
}

/*
 * Signal handlers
 */

static gboolean
when_save_timeout(gpointer data)
{
	OckStationList *self = OCK_STATION_LIST(data);
	OckStationListPrivate *priv = self->priv;

	ock_station_list_save(self);

	priv->save_timeout_id = 0;

	return G_SOURCE_REMOVE;
}

static void
ock_station_list_schedule_save(OckStationList *self)
{
	OckStationListPrivate *priv = self->priv;

	if (priv->save_timeout_id > 0)
		g_source_remove(priv->save_timeout_id);

	priv->save_timeout_id =
	        g_timeout_add_seconds(SAVE_DELAY, when_save_timeout, self);
}

static void
on_station_notify(OckStation     *station,
                  GParamSpec     *pspec,
                  OckStationList *self)
{
	const gchar *property_name = g_param_spec_get_name(pspec);

	TRACE("%s, %s, %p", ock_station_get_uid(station), property_name, self);

	/* We might want to save changes */
	if (pspec->flags & GSZN_PARAM_SERIALIZE)
		ock_station_list_schedule_save(self);

	/* Emit signal */
	g_signal_emit(self, signals[SIGNAL_STATION_MODIFIED], 0, station);
}

/*
 * Public functions
 */

void
ock_station_list_remove(OckStationList *self, OckStation *station)
{
	OckStationListPrivate *priv = self->priv;
	GList *item;

	/* Ensure a valid station was given */
	if (station == NULL) {
		WARNING("Attempting to remove NULL station");
		return;
	}

	/* Give info */
	INFO("Removing station '%s'", ock_station_get_name_or_uri(station));

	/* Check that we own this station at first. If we don't find it
	 * in our internal list, it's probably a programming error.
	 */
	item = g_list_find(priv->stations, station);
	if (item == NULL) {
		WARNING("OckStation %p (%s) not found in list",
		        station, ock_station_get_uid(station));
		return;
	}

	/* Disconnect signal handlers */
	g_signal_handlers_disconnect_by_data(station, self);

	/* Remove from list */
	priv->stations = g_list_remove_link(priv->stations, item);
	g_list_free(item);

	/* Unown the station */
	g_object_unref(station);

	/* Rebuild the shuffled station list */
	if (priv->shuffled) {
		g_list_free_full(priv->shuffled, g_object_unref);
		priv->shuffled = g_list_copy_deep_shuffle(priv->stations,
		                 (GCopyFunc) g_object_ref, NULL);
	}

	/* Emit a signal */
	g_signal_emit(self, signals[SIGNAL_STATION_REMOVED], 0, station);

	/* Save */
	ock_station_list_schedule_save(self);
}

void
ock_station_list_insert(OckStationList *self, OckStation *station, gint pos)
{
	OckStationListPrivate *priv = self->priv;
	GList *similar_item;

	/* Ensure a valid station was given */
	if (station == NULL) {
		WARNING("Attempting to insert NULL station");
		return;
	}

	/* Give info */
	INFO("Inserting station '%s'", ock_station_get_name_or_uri(station));

	/* Check that the station is not already part of the list.
	 * Duplicates are a programming error, we must warn about that.
	 * Identical fields are an user error.
	 * Warnings and such are encapsulated in the GCompareFunc used
	 * here, this is messy but temporary (hopefully).
	 */
	similar_item = g_list_find_custom(priv->stations, station,
	                                  (GCompareFunc) are_stations_similar);
	if (similar_item)
		return;

	/* We own the station now */
	g_object_ref(station);

	/* Add to the list at the right position */
	priv->stations = g_list_insert(priv->stations, station, pos);

	/* Connect to notify signal */
	g_signal_connect(station, "notify", G_CALLBACK(on_station_notify), self);

	/* Rebuild the shuffled station list */
	if (priv->shuffled) {
		g_list_free_full(priv->shuffled, g_object_unref);
		priv->shuffled = g_list_copy_deep_shuffle(priv->stations,
		                 (GCopyFunc) g_object_ref, NULL);
	}

	/* Emit a signal */
	g_signal_emit(self, signals[SIGNAL_STATION_ADDED], 0, station);

	/* Save */
	ock_station_list_schedule_save(self);
}

/* Insert a station before another.
 * If 'before' is NULL or is not found, the station is appended at the end of the list.
 */
void
ock_station_list_insert_before(OckStationList *self, OckStation *station, OckStation *before)
{
	OckStationListPrivate *priv = self->priv;
	gint pos = -1;

	pos = g_list_index(priv->stations, before);
	ock_station_list_insert(self, station, pos);
}

/* Insert a station after another.
 * If 'after' is NULL or not found, the station is appended at the beginning of the list.
 */
void
ock_station_list_insert_after(OckStationList *self, OckStation *station, OckStation *after)
{
	OckStationListPrivate *priv = self->priv;
	gint pos = 0;

	pos = g_list_index(priv->stations, after);
	pos += 1; // tricky but does what we want even for pos == -1
	ock_station_list_insert(self, station, pos);
}

void
ock_station_list_prepend(OckStationList *self, OckStation *station)
{
	ock_station_list_insert_after(self, station, NULL);
}

void
ock_station_list_append(OckStationList *self, OckStation *station)
{
	ock_station_list_insert_before(self, station, NULL);
}

void
ock_station_list_move(OckStationList *self, OckStation *station, gint pos)
{
	OckStationListPrivate *priv = self->priv;
	GList *item;

	/* Find the station */
	item = g_list_find(priv->stations, station);
	if (item == NULL) {
		WARNING("OckStation %p (%s) not found in list",
		        station, ock_station_get_uid(station));
		return;
	}

	/* Move it */
	priv->stations = g_list_remove_link(priv->stations, item);
	g_list_free(item);
	priv->stations = g_list_insert(priv->stations, station, pos);

	/* Emit a signal */
	g_signal_emit(self, signals[SIGNAL_STATION_MOVED], 0, station);

	/* Save */
	ock_station_list_schedule_save(self);
}

/* Move a station before another.
 * If 'before' is NULL or not found, the station is inserted at the end of the list.
 */
void
ock_station_list_move_before(OckStationList *self, OckStation *station, OckStation *before)
{
	OckStationListPrivate *priv = self->priv;
	gint pos = -1;

	pos = g_list_index(priv->stations, before);
	ock_station_list_move(self, station, pos);
}

void
ock_station_list_move_after(OckStationList *self, OckStation *station, OckStation *after)
{
	OckStationListPrivate *priv = self->priv;
	gint pos = 0;

	pos = g_list_index(priv->stations, after);
	pos += 1; // tricky but does what we want even for pos == -1
	ock_station_list_move(self, station, pos);
}

void
ock_station_list_move_first(OckStationList *self, OckStation *station)
{
	ock_station_list_move_after(self, station, NULL);
}

void
ock_station_list_move_last(OckStationList *self, OckStation *station)
{
	ock_station_list_move_before(self, station, NULL);
}

OckStation *
ock_station_list_prev(OckStationList *self, OckStation *station,
                      gboolean repeat, gboolean shuffle)
{
	OckStationListPrivate *priv = self->priv;
	GList *stations, *item;

	/* Pickup the right station list, create shuffle list if needed */
	if (shuffle) {
		if (priv->shuffled == NULL)
			priv->shuffled = g_list_copy_deep_shuffle(priv->stations,
			                 (GCopyFunc) g_object_ref, NULL);
		stations = priv->shuffled;
	} else {
		if (priv->shuffled) {
			g_list_free_full(priv->shuffled, g_object_unref);
			priv->shuffled = NULL;
		}
		stations = priv->stations;
	}

	/* Return last station for NULL argument */
	if (station == NULL)
		return g_list_last(stations)->data;

	/* Try to find station in station list */
	item = g_list_find(stations, station);
	if (item == NULL)
		return NULL;

	/* Return previous station if any */
	item = item->prev;
	if (item)
		return item->data;

	/* Without repeat, there's no more station */
	if (!repeat)
		return NULL;

	/* With repeat, we may re-shuffle, then return the last station */
	if (shuffle) {
		priv->shuffled = g_list_shuffle(priv->shuffled);
		stations = priv->shuffled;
		// TODO Ensure that the last station is not the current station
	}

	return g_list_last(stations)->data;
}

OckStation *
ock_station_list_next(OckStationList *self, OckStation *station,
                      gboolean repeat, gboolean shuffle)
{
	OckStationListPrivate *priv = self->priv;
	GList *stations, *item;

	/* Pickup the right station list, create shuffle list if needed */
	if (shuffle) {
		if (priv->shuffled == NULL)
			priv->shuffled = g_list_copy_deep_shuffle(priv->stations,
			                 (GCopyFunc) g_object_ref, NULL);
		stations = priv->shuffled;
	} else {
		if (priv->shuffled) {
			g_list_free_full(priv->shuffled, g_object_unref);
			priv->shuffled = NULL;
		}
		stations = priv->stations;
	}

	/* Return first station for NULL argument */
	if (station == NULL)
		return stations->data;

	/* Try to find station in station list */
	item = g_list_find(stations, station);
	if (item == NULL)
		return NULL;

	/* Return next station if any */
	item = item->next;
	if (item)
		return item->data;

	/* Without repeat, there's no more station */
	if (!repeat)
		return NULL;

	/* With repeat, we may re-shuffle, then return the first station */
	if (shuffle) {
		priv->shuffled = g_list_shuffle(priv->shuffled);
		stations = priv->shuffled;
	}

	return stations->data;
}

OckStation *
ock_station_list_first(OckStationList *self)
{
	GList *stations = self->priv->stations;

	if (stations == NULL)
		return NULL;

	return g_list_first(stations)->data;
}

OckStation *
ock_station_list_last(OckStationList *self)
{
	GList *stations = self->priv->stations;

	if (stations == NULL)
		return NULL;

	return g_list_last(stations)->data;
}

OckStation *
ock_station_list_find(OckStationList *self, OckStation *station)
{
	GList *stations = self->priv->stations;
	GList *item;

	item = g_list_find(stations, station);

	return item ? item->data : NULL;
}

OckStation *
ock_station_list_find_by_name(OckStationList *self, const gchar *name)
{
	GList *item;

	/* Ensure station name is valid */
	if (name == NULL) {
		WARNING("Attempting to find a station with NULL name");
		return NULL;
	}

	/* Forbid empty names */
	if (!g_strcmp0(name, ""))
		return NULL;

	/* Iterate on station list */
	for (item = self->priv->stations; item; item = item->next) {
		OckStation *station = item->data;
		if (!g_strcmp0(name, ock_station_get_name(station)))
			return station;
	}

	return NULL;
}

OckStation *
ock_station_list_find_by_uri(OckStationList *self, const gchar *uri)
{
	GList *item;

	/* Ensure station name is valid */
	if (uri == NULL) {
		WARNING("Attempting to find a station with NULL uri");
		return NULL;
	}

	/* Iterate on station list */
	for (item = self->priv->stations; item; item = item->next) {
		OckStation *station = item->data;
		if (!g_strcmp0(uri, ock_station_get_uri(station)))
			return station;
	}

	return NULL;
}

OckStation *
ock_station_list_find_by_uid(OckStationList *self, const gchar *uid)
{
	GList *item;

	/* Ensure station name is valid */
	if (uid == NULL) {
		WARNING("Attempting to find a station with NULL uid");
		return NULL;
	}

	/* Iterate on station list */
	for (item = self->priv->stations; item; item = item->next) {
		OckStation *station = item->data;
		if (!g_strcmp0(uid, ock_station_get_uid(station)))
			return station;
	}

	return NULL;
}

OckStation  *
ock_station_list_find_by_guessing(OckStationList *self, const gchar *string)
{
	if (is_uri_scheme_supported(string))
		return ock_station_list_find_by_uri(self, string);
	else
		/* Assume it's the station name */
		return ock_station_list_find_by_name(self, string);
}

void
ock_station_list_save(OckStationList *self)
{
	OckStationListPrivate *priv = self->priv;
	GsznSerializer *serializer;
	GError *err = NULL;
	gchar *text;

	/* Create a serializer */
	serializer = gszn_serializer_new(priv->serialization_settings,
	                                 GSZN_SERIALIZER_FLAG_SERIALIZE_FLAG_ONLY |
	                                 GSZN_SERIALIZER_FLAG_NON_DEFAULT_ONLY);

	gszn_serializer_add_list(serializer, priv->stations);

	/* Stringify data */
	text = gszn_serializer_print(serializer);

	/* Write to file */
	ock_file_write_sync(priv->save_path, text, &err);

	/* Cleanup */
	g_free(text);
	g_object_unref(serializer);

	/* Handle error */
	if (err == NULL) {
		INFO("Station list saved to '%s'", priv->save_path);
	} else {
		gchar *str;

		str = g_strdup_printf("Failed to save station list: %s", err->message);

		INFO("%s", str);
		ock_errorable_emit_error(OCK_ERRORABLE(self), str);

		g_free(str);
		g_clear_error(&err);
	}
}

void
ock_station_list_load(OckStationList *self)
{
	OckStationListPrivate *priv = self->priv;
	GsznDeserializer *deserializer;
	GSList *item = NULL;
	GList *sta_item;

	TRACE("%p", self);

	/* This should be called only once at startup */
	g_assert(priv->stations == NULL);

	/* Create the deserializer */
	deserializer = gszn_deserializer_new(priv->serialization_settings);

	/* Load from a list of pathes */
	for (item = priv->load_pathes; item; item = item->next) {
		GError *err = NULL;
		const gchar *path = item->data;
		gchar *text;

		/* Attempt to read file */
		ock_file_read_sync(path, &text, &err);
		if (err) {
			WARNING("%s", err->message);
			g_clear_error(&err);
			continue;
		}

		/* Attempt to parse it */
		gszn_deserializer_parse(deserializer, text, &err);
		g_free(text);
		if (err) {
			WARNING("Failed to parse '%s': %s", path, err->message);
			g_clear_error(&err);
			continue;
		}

		/* Success */
		break;
	}

	/* Check if we got something */
	if (item) {
		const gchar *loaded_path = item->data;

		INFO("Station list loaded from file '%s'", loaded_path);
	} else {
		GError *err = NULL;

		INFO("No valid station list file found, using hard-coded default");

		gszn_deserializer_parse(deserializer, DEFAULT_STATION_LIST, &err);
		if (err) {
			ERROR("%s", err->message);
			/* Program execution stops here */
		}
	}

	/* Now it's time to create the stations */
	priv->stations = gszn_deserializer_create_all(deserializer);

	/* Dispose of the deserializer */
	g_object_unref(deserializer);

	/* Register a notify handler for each station */
	for (sta_item = priv->stations; sta_item; sta_item = sta_item->next) {
		OckStation *station = sta_item->data;

		g_signal_connect(station, "notify", G_CALLBACK(on_station_notify), self);
	}
}

OckStationList *
ock_station_list_new(void)
{
	return g_object_new(OCK_TYPE_STATION_LIST, NULL);
}

/*
 * GObject methods
 */

static void
ock_station_list_finalize(GObject *object)
{
	OckStationList *self = OCK_STATION_LIST(object);
	OckStationListPrivate *priv = self->priv;
	GList *item;

	TRACE("%p", object);

	/* Run any pending save operation */
	if (priv->save_timeout_id > 0)
		when_save_timeout(self);

	/* Disconnect stations signal handlers */
	for (item = priv->stations; item; item = item->next) {
		OckStation *station = item->data;
		g_signal_handlers_disconnect_by_data(station, self);
	}

	/* Free station lists */
	g_list_free_full(priv->stations, g_object_unref);
	g_list_free_full(priv->shuffled, g_object_unref);

	/* Free serialization settings */
	gszn_settings_free(priv->serialization_settings);

	/* Drop the reference to the station type */
	g_type_class_unref(g_type_class_peek(OCK_TYPE_STATION));

	/* Free pathes */
	g_free(priv->save_path);
	g_slist_free_full(priv->load_pathes, g_free);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_station_list, object);
}

static void
ock_station_list_constructed(GObject *object)
{
	OckStationList *self = OCK_STATION_LIST(object);
	OckStationListPrivate *priv = self->priv;
	GsznSettings *settings;

	/* Initialize pathes */
	priv->load_pathes = ock_get_existing_path_list
	                    (OCK_DIR_USER_CONFIG | OCK_DIR_SYSTEM_CONFIG, "stations");
	priv->save_path = g_build_filename(ock_get_user_config_dir(), "stations", NULL);

	/* Ensure the station type is registered in the type system
	 * (this is needed when deserializing the station list)
	 */
	g_type_class_ref(OCK_TYPE_STATION);

	/* Create serialization settings */
	settings = gszn_settings_new();
	settings->backend_type        = GSZN_TYPE_BACKEND_XML;
	settings->title               = "Stations";
	settings->ser_object_name     = serialize_object_name;
	settings->deser_object_name   = deserialize_object_name;
	priv->serialization_settings  = settings;

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_station_list, object);
}

static void
ock_station_list_init(OckStationList *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_station_list_get_instance_private(self);
}

static void
ock_station_list_class_init(OckStationListClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_station_list_finalize;
	object_class->constructed = ock_station_list_constructed;

	/* Signals */
	signals[SIGNAL_STATION_ADDED] =
	        g_signal_new("station-added", G_TYPE_FROM_CLASS(class),
	                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                     0, NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE,
	                     1, G_TYPE_OBJECT);

	signals[SIGNAL_STATION_REMOVED] =
	        g_signal_new("station-removed", G_TYPE_FROM_CLASS(class),
	                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                     0, NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE,
	                     1, G_TYPE_OBJECT);

	signals[SIGNAL_STATION_MODIFIED] =
	        g_signal_new("station-modified", G_TYPE_FROM_CLASS(class),
	                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                     0, NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE,
	                     1, G_TYPE_OBJECT);

	signals[SIGNAL_STATION_MOVED] =
	        g_signal_new("station-moved", G_TYPE_FROM_CLASS(class),
	                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                     0, NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE,
	                     1, G_TYPE_OBJECT);
}
