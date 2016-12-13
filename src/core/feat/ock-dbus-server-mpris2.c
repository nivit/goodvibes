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

#include <string.h>
#include <math.h>

#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>

#include "additions/glib.h"
#include "additions/glib-object.h"

#include "framework/log.h"
#include "framework/uri-schemes.h"
#include "framework/ock-framework.h"

#include "core/ock-core.h"

#include "core/feat/ock-dbus-server.h"
#include "core/feat/ock-dbus-server-mpris2.h"

#define TRACKID_PATH         "/org/" PACKAGE_CAMEL_NAME "/StationList"

#define DBUS_NAME            "org.mpris.MediaPlayer2." PACKAGE_CAMEL_NAME
#define DBUS_PATH            "/org/mpris/MediaPlayer2"
#define DBUS_IFACE_ROOT      "org.mpris.MediaPlayer2"
#define DBUS_IFACE_PLAYER    DBUS_IFACE_ROOT ".Player"
#define DBUS_IFACE_TRACKLIST DBUS_IFACE_ROOT ".TrackList"

static const gchar *DBUS_INTROSPECTION =
        "<node>"
        "    <interface name='"DBUS_IFACE_ROOT"'>"
        "        <method name='Raise'/>"
        "        <method name='Quit'/>"
        "        <property name='CanRaise'            type='b'  access='read'/>"
        "        <property name='CanQuit'             type='b'  access='read'/>"
        "        <property name='Fullscreen'          type='b'  access='readwrite'/>"
        "        <property name='CanSetFullscreen'    type='b'  access='read'/>"
        "        <property name='HasTrackList'        type='b'  access='read'/>"
        "        <property name='Identity'            type='s'  access='read'/>"
        "        <property name='DesktopEntry'        type='s'  access='read'/>"
        "        <property name='SupportedUriSchemes' type='as' access='read'/>"
        "        <property name='SupportedMimeTypes'  type='as' access='read'/>"
        "    </interface>"
        "    <interface name='"DBUS_IFACE_PLAYER"'>"
        "        <method name='Play'/>"
        "        <method name='Pause'/>"
        "        <method name='PlayPause'/>"
        "        <method name='Stop'/>"
        "        <method name='Next'/>"
        "        <method name='Previous'/>"
        "        <method name='Seek'>"
        "            <arg direction='in' name='Offset' type='x'/>"
        "        </method>"
        "        <method name='SetPosition'>"
        "            <arg direction='in' name='TrackId'  type='o'/>"
        "            <arg direction='in' name='Position' type='x'/>"
        "        </method>"
        "        <method name='OpenUri'>"
        "            <arg direction='in' name='Uri' type='s'/>"
        "        </method>"
        "        <signal name='Seeked'>"
        "            <arg name='Position' type='x'/>"
        "        </signal>"
        "        <property name='PlaybackStatus' type='s'     access='read'/>"
        "        <property name='LoopStatus'     type='s'     access='readwrite'/>"
        "        <property name='Shuffle'        type='b'     access='readwrite'/>"
        "        <property name='Volume'         type='d'     access='readwrite'/>"
        "        <property name='Rate'           type='d'     access='readwrite'/>"
        "        <property name='MinimumRate'    type='d'     access='read'/>"
        "        <property name='MaximumRate'    type='d'     access='read'/>"
        "        <property name='Metadata'       type='a{sv}' access='read'/>"
        "        <property name='CanPlay'        type='b'     access='read'/>"
        "        <property name='CanPause'       type='b'     access='read'/>"
        "        <property name='CanGoNext'      type='b'     access='read'/>"
        "        <property name='CanGoPrevious'  type='b'     access='read'/>"
        "        <property name='CanSeek'        type='b'     access='read'/>"
        "        <property name='CanControl'     type='b'     access='read'/>"
        "    </interface>"
        "    <interface name='"DBUS_IFACE_TRACKLIST"'>"
        "        <method name='GetTracksMetadata'>"
        "            <arg direction='in'  name='TrackIds'     type='ao'/>"
        "            <arg direction='out' name='Metadata'     type='aa{sv}'/>"
        "        </method>"
        "        <method name='AddTrack'>"
        "            <arg direction='in'  name='Uri'          type='s'/>"
        "            <arg direction='in'  name='AfterTrack'   type='o'/>"
        "            <arg direction='in'  name='SetAsCurrent' type='b'/>"
        "        </method>"
        "        <method name='RemoveTrack'>"
        "            <arg direction='in'  name='TrackId'      type='o'/>"
        "        </method>"
        "        <method name='GoTo'>"
        "            <arg direction='in'  name='TrackId'      type='o'/>"
        "        </method>"
        "        <signal name='TrackListReplaced'>"
        "            <arg name='Tracks'       type='ao'/>"
        "            <arg name='CurrentTrack' type='o'/>"
        "        </signal>"
        "        <signal name='TrackAdded'>"
        "            <arg name='Metadata'     type='a{sv}'/>"
        "            <arg name='AfterTrack'   type='o'/>"
        "        </signal>"
        "        <signal name='TrackRemoved'>"
        "            <arg name='TrackId'      type='o'/>"
        "        </signal>"
        "        <signal name='TrackMetadataChanged'>"
        "            <arg name='TrackId'      type='o'/>"
        "            <arg name='Metadata'     type='a{sv}'/>"
        "        </signal>"
        "        <property name='Tracks'        type='ao' access='read'/>"
        "        <property name='CanEditTracks' type='b'  access='read'/>"
        "    </interface>"
        "</node>";

/*
 * GObject definitions
 */

struct _OckDbusServerMpris2 {
	/* Parent instance structure */
	OckDbusServer parent_instance;
};

G_DEFINE_TYPE(OckDbusServerMpris2, ock_dbus_server_mpris2, OCK_TYPE_DBUS_SERVER)

/*
 * Helpers
 */

static gchar *
make_track_id(OckStation *station)
{
	if (station == NULL)
		return g_strdup(DBUS_PATH "/TrackList/NoTrack");

	return g_strdup_printf(TRACKID_PATH "/%s", ock_station_get_uid(station));
}

static gboolean
parse_track_id(const gchar     *track_id,
               OckStationList  *station_list,
               OckStation     **station)
{
	const gchar *station_uid;

	g_assert(station != NULL);
	*station = NULL;

	if (!g_strcmp0(track_id, DBUS_PATH "/TrackList/NoTrack"))
		return TRUE;

	if (!g_str_has_prefix(track_id, TRACKID_PATH "/"))
		return FALSE;

	station_uid = track_id + strlen(TRACKID_PATH "/");
	*station = ock_station_list_find_by_uid(station_list, station_uid);
	if (*station == NULL)
		return FALSE;

	return TRUE;
}

static GVariant *
g_variant_new_metadata(OckStation *station, OckMetadata *metadata)
{
	GVariantBuilder b;
	gchar *track_id;
	const gchar *uri;
	gchar *artist;
	gchar *title;
	gchar *album;
	gchar *genre;
	gchar *year;
	gchar *comment;

	g_variant_builder_init(&b, G_VARIANT_TYPE("a{sv}"));

	/* Station properties */
	if (station == NULL)
		goto end;

	track_id = make_track_id(station);
	g_variant_builder_add_dictentry_object_path(&b, "mpris:trackid", track_id);
	g_free(track_id);

	uri = ock_station_get_uri(station);
	g_variant_builder_add_dictentry_string(&b, "xesam:url", uri);

	/* Metadata if any */
	if (metadata == NULL)
		goto end;

	g_object_get(metadata,
	             "artist", &artist,
	             "title", &title,
	             "album", &album,
	             "genre", &genre,
	             "year", &year,
	             "comment", &comment,
	             NULL);

	if (artist)
		g_variant_builder_add_dictentry_array_string(&b, "xesam:artist", artist, NULL);
	if (title)
		g_variant_builder_add_dictentry_string(&b, "xesam:title", title);
	if (album)
		g_variant_builder_add_dictentry_string(&b, "xesam:album", album);
	if (genre)
		g_variant_builder_add_dictentry_array_string(&b, "xesam:genre", genre, NULL);
	if (year)
		g_variant_builder_add_dictentry_string(&b, "xesam:contentCreated", year);
	if (comment)
		g_variant_builder_add_dictentry_array_string(&b, "xesam:comment", comment, NULL);

	g_free(artist);
	g_free(title);
	g_free(album);
	g_free(genre);
	g_free(year);
	g_free(comment);

end:
	return g_variant_builder_end(&b);
}

static GVariant *
g_variant_new_playback_status(OckPlayer *player)
{
	OckPlayerState state;
	gchar *state_str;

	state = ock_player_get_state(player);

	switch (state) {
	case OCK_PLAYER_STATE_PLAYING:
		state_str = "Playing";
		break;
	case OCK_PLAYER_STATE_STOPPED:
	default:
		state_str = "Stopped";
		break;
	}

	return g_variant_new_string(state_str);
}

static GVariant *
g_variant_new_loop_status(OckPlayer *player)
{
	gboolean repeat;

	repeat = ock_player_get_repeat(player);
	return g_variant_new_string(repeat ? "Playlist" : "None");
}

static GVariant *
g_variant_new_shuffle(OckPlayer *player)
{
	gboolean shuffle;

	shuffle = ock_player_get_shuffle(player);
	return g_variant_new_boolean(shuffle);
}

static GVariant *
g_variant_new_volume(OckPlayer *player)
{
	guint volume;

	volume = ock_player_get_volume(player);
	return g_variant_new_double((gdouble) volume / 100.0);
}

/*
 * Dbus method handlers
 */

static GVariant *
method_raise(OckDbusServer  *dbus_server G_GNUC_UNUSED,
             GVariant       *params G_GNUC_UNUSED,
             GError        **error G_GNUC_UNUSED)
{
	return NULL;
}

static GVariant *
method_quit(OckDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	ock_framework_quit_loop();

	return NULL;
}

static OckDbusMethod root_methods[] = {
	{ "Raise", method_raise },
	{ "Quit",  method_quit  },
	{ NULL,    NULL         }
};

static GVariant *
method_play(OckDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;

	ock_player_play(player);

	return NULL;
}

static GVariant *
method_stop(OckDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;

	ock_player_stop(player);

	return NULL;
}

static GVariant *
method_toggle(OckDbusServer  *dbus_server G_GNUC_UNUSED,
              GVariant       *params G_GNUC_UNUSED,
              GError        **error G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;

	ock_player_toggle(player);

	return NULL;
}

static GVariant *
method_next(OckDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;

	if (!ock_player_next(player))
		ock_player_stop(player);

	return NULL;
}

static GVariant *
method_prev(OckDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;

	if (!ock_player_prev(player))
		ock_player_stop(player);

	return NULL;
}

static GVariant *
method_open_uri(OckDbusServer  *dbus_server G_GNUC_UNUSED,
                GVariant       *params G_GNUC_UNUSED,
                GError        **error G_GNUC_UNUSED)
{
	OckPlayer *player  = ock_core_player;
	OckStationList *station_list = ock_core_station_list;
	OckStation *station;
	const gchar *uri;

	g_variant_get(params, "(&s)", &uri);

	/* Ensure URI is valid */
	if (!is_uri_scheme_supported(uri)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "URI scheme not supported.");
		return NULL;
	}

	/* Check if the station is part of the station list */
	station = ock_station_list_find_by_uri(station_list, uri);

	/* Create a new station if needed */
	if (station == NULL) {
		station = ock_station_new(NULL, uri);
		ock_station_list_append(station_list, station);
		g_object_unref(station);
	}

	/* Play station */
	ock_player_set_station(player, station);
	ock_player_play(player);

	return NULL;
}

static OckDbusMethod player_methods[] = {
	{ "Play",        method_play     },
	{ "Pause",       method_stop     },
	{ "PlayPause",   method_toggle   },
	{ "Stop",        method_stop     },
	{ "Next",        method_next     },
	{ "Previous",    method_prev     },
	{ "Seek",        NULL            },
	{ "SetPosition", NULL            },
	{ "OpenUri",     method_open_uri },
	{ NULL,          NULL            }
};

static GVariant *
method_get_tracks_metadata(OckDbusServer  *dbus_server G_GNUC_UNUSED,
                           GVariant       *params,
                           GError        **error G_GNUC_UNUSED)
{
	OckStationList *station_list = ock_core_station_list;
	GVariantBuilder b;
	GVariantIter *iter;
	const gchar *track_id;

	g_variant_get(params, "(ao)", &iter);
	g_variant_builder_init(&b, G_VARIANT_TYPE("aa{sv}"));
	while (g_variant_iter_loop(iter, "&o", &track_id)) {
		OckStation *station;

		if (!parse_track_id(track_id, station_list, &station))
			/* Ignore silently */
			continue;

		g_variant_builder_add_value(&b, g_variant_new_metadata(station, NULL));
	}

	return g_variant_builder_end(&b);
}

static GVariant *
method_add_track(OckDbusServer  *dbus_server G_GNUC_UNUSED,
                 GVariant       *params,
                 GError        **error)
{
	OckPlayer *player = ock_core_player;
	OckStationList *station_list = ock_core_station_list;
	const gchar *uri;
	const gchar *after_track;
	gboolean set_as_current;
	OckStation *station, *after_station;

	g_variant_get(params, "(&s&ob)", &uri, &after_track, &set_as_current);

	/* Ensure URI is valid */
	if (!is_uri_scheme_supported(uri)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid URI scheme for param 'Uri'.");
		return NULL;
	}

	/* Handle after track */
	if (!parse_track_id(after_track, station_list, &after_station)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid param 'AfterTrack'.");
		return NULL;
	}

	/* Add a new station to station list. If after_station is NULL,
	 * MPRIS2 indicates that the track should be placed at the
	 * beginning of the track list.
	 */
	station = ock_station_new(NULL, uri);

	if (after_station)
		ock_station_list_insert_after(station_list, station, after_station);
	else
		ock_station_list_prepend(station_list, station);

	g_object_unref(station);

	/* Play new station if needed */
	if (set_as_current) {
		ock_player_set_station(player, station);
		if (ock_player_get_state(player) != OCK_PLAYER_STATE_STOPPED)
			ock_player_play(player);
	}

	return NULL;
}

static GVariant *
method_remove_track(OckDbusServer  *dbus_server G_GNUC_UNUSED,
                    GVariant       *params,
                    GError        **error)
{
	OckStationList *station_list = ock_core_station_list;
	const gchar *track_id;
	OckStation *station;

	g_variant_get(params, "(&o)", &track_id);
	if (!parse_track_id(track_id, station_list, &station)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid param 'TrackId'.");
		return NULL;
	}

	ock_station_list_remove(station_list, station);

	return NULL;
}

static GVariant *
method_go_to(OckDbusServer  *dbus_server G_GNUC_UNUSED,
             GVariant       *params,
             GError        **error)
{
	OckPlayer *player = ock_core_player;
	OckStationList *station_list = ock_core_station_list;
	const gchar *track_id;
	OckStation *station;

	// WISHED What about the last line in MPRIS2 specs ? What does that mean ?

	g_variant_get(params, "(&o)", &track_id);
	if (!parse_track_id(track_id, station_list, &station)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid param 'TrackId'.");
		return NULL;
	}

	ock_player_set_station(player, station);

	if (ock_player_get_state(player) != OCK_PLAYER_STATE_STOPPED)
		ock_player_play(player);

	return NULL;
}

static OckDbusMethod tracklist_methods[] = {
	{ "GetTracksMetadata", method_get_tracks_metadata },
	{ "AddTrack",          method_add_track           },
	{ "RemoveTrack",       method_remove_track        },
	{ "GoTo",              method_go_to               },
	{ NULL,                NULL                       }
};

/*
 * Dbus property handlers
 */

static GVariant *
prop_get_true(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	/* Dummy accessor to return TRUE all the time */
	return g_variant_new_boolean(TRUE);
}

static GVariant *
prop_get_false(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	/* Dummy accessor to return FALSE all the time */
	return g_variant_new_boolean(FALSE);
}

static gboolean
prop_set_error(OckDbusServer  *dbus_server G_GNUC_UNUSED,
               GVariant       *value G_GNUC_UNUSED,
               GError        **error)
{
	g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
	            "Setting this property is not supported.");

	return FALSE;
}

static GVariant *
prop_get_identity(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	// TODO Add gettext stuff for translating, I saw that somewhere.
	//      But, wait, what does the MPRIS2 spec say about that ?
	return g_variant_new_string(PACKAGE_LONG_NAME);
}

static GVariant *
prop_get_desktop_entry(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	return g_variant_new_string(PACKAGE_NAME);
}

static GVariant *
prop_get_supported_uri_schemes(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	return g_variant_new_strv(SUPPORTED_URI_SCHEMES, -1);
}

static GVariant *
prop_get_supported_mime_types(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	return g_variant_new_strv(SUPPORTED_MIME_TYPES, -1);
}

static OckDbusProperty root_properties[] = {
	{ "CanRaise",            prop_get_false,                 NULL },
	{ "CanQuit",             prop_get_true,                  NULL },
	{ "Fullscreen",          prop_get_false,                 prop_set_error },
	{ "CanSetFullscreen",    prop_get_false,                 NULL },
	{ "HasTrackList",        prop_get_true,                  NULL },
	{ "Identity",            prop_get_identity,              NULL },
	{ "DesktopEntry",        prop_get_desktop_entry,         NULL },
	{ "SupportedUriSchemes", prop_get_supported_uri_schemes, NULL },
	{ "SupportedMimeTypes",  prop_get_supported_mime_types,  NULL },
	{ NULL,                  NULL,                           NULL }
};

static GVariant *
prop_get_playback_status(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;

	return g_variant_new_playback_status(player);
}

static GVariant *
prop_get_loop_status(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;

	return g_variant_new_loop_status(player);
}

static gboolean
prop_set_loop_status(OckDbusServer  *dbus_server G_GNUC_UNUSED,
                     GVariant       *value,
                     GError        **error)
{
	OckPlayer *player = ock_core_player;
	const gchar *loop_status;
	gboolean repeat;

	repeat = FALSE;
	loop_status = g_variant_get_string(value, NULL);

	if (!g_strcmp0(loop_status, "Playlist")) {
		repeat = TRUE;
	} else if (!g_strcmp0(loop_status, "Track")) {
		/* 'Track' makes no sense here */
		repeat = FALSE;
	} else if (!g_strcmp0(loop_status, "None")) {
		repeat = FALSE;
	} else {
		/* Any other value should raise an error */
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid value.");

		return FALSE;
	}

	ock_player_set_repeat(player, repeat);

	return TRUE;
}

static GVariant *
prop_get_shuffle(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;

	return g_variant_new_shuffle(player);
}

static gboolean
prop_set_shuffle(OckDbusServer  *dbus_server G_GNUC_UNUSED,
                 GVariant       *value,
                 GError        **error G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;
	gboolean shuffle;

	shuffle = g_variant_get_boolean(value);
	ock_player_set_shuffle(player, shuffle);

	return TRUE;
}

static GVariant *
prop_get_volume(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;

	return g_variant_new_volume(player);
}

static gboolean
prop_set_volume(OckDbusServer  *dbus_server G_GNUC_UNUSED,
                GVariant       *value,
                GError        **error G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;
	gdouble volume;

	volume = g_variant_get_double(value);
	volume = round(volume * 100);
	ock_player_set_volume(player, (guint) volume);

	return TRUE;
}

static GVariant *
prop_get_rate(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	return g_variant_new_double(1.0);
}

static GVariant *
prop_get_metadata(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;
	OckStation *station;
	OckMetadata *metadata;

	station = ock_player_get_station(player);
	metadata = ock_player_get_metadata(player);

	return g_variant_new_metadata(station, metadata);
}

static GVariant *
prop_has_current(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;
	gboolean has_current;

	has_current = ock_player_get_station(player) ? TRUE : FALSE;

	return g_variant_new_boolean(has_current);
}

static GVariant *
prop_has_prev(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;
	gboolean has_prev;

	has_prev = ock_player_get_prev_station(player) ? TRUE : FALSE;

	return g_variant_new_boolean(has_prev);
}

static GVariant *
prop_has_next(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;
	gboolean has_next;

	has_next = ock_player_get_next_station(player) ? TRUE : FALSE;

	return g_variant_new_boolean(has_next);
}

static OckDbusProperty player_properties[] = {
	{ "PlaybackStatus", prop_get_playback_status, NULL },
	{ "LoopStatus",     prop_get_loop_status,     prop_set_loop_status },
	{ "Shuffle",        prop_get_shuffle,         prop_set_shuffle },
	{ "Volume",         prop_get_volume,          prop_set_volume },
	{ "Rate",           prop_get_rate,            prop_set_error },
	{ "MinimumRate",    prop_get_rate,            NULL },
	{ "MaximumRate",    prop_get_rate,            NULL },
	{ "Metadata",       prop_get_metadata,        NULL },
	{ "CanPlay",        prop_has_current,         NULL },
	{ "CanPause",       prop_has_current,         NULL },
	{ "CanGoNext",      prop_has_next,            NULL },
	{ "CanGoPrevious",  prop_has_prev,            NULL },
	{ "CanSeek",        prop_get_false,           NULL },
	{ "CanControl",     prop_get_true,            NULL },
	{ NULL,             NULL,                     NULL }
};

static GVariant *
prop_get_tracks(OckDbusServer *dbus_server G_GNUC_UNUSED)
{
	OckStationList *station_list = ock_core_station_list;
	OckStationListIter *iter;
	OckStation *station;
	GVariantBuilder b;

	g_variant_builder_init(&b, G_VARIANT_TYPE ("ao"));
	iter = ock_station_list_iter_new(station_list);

	while (ock_station_list_iter_loop(iter, &station)) {
		gchar *track_id;
		track_id = make_track_id(station);
		g_variant_builder_add(&b, "o", track_id);
		g_free(track_id);
	}

	ock_station_list_iter_free(iter);
	return g_variant_builder_end(&b);
}

static OckDbusProperty tracklist_properties[] = {
	{ "Tracks",        prop_get_tracks, NULL },
	{ "CanEditTracks", prop_get_true,   NULL },
	{ NULL,            NULL,            NULL }
};

/*
 * Dbus interfaces
 */

static OckDbusInterface dbus_interfaces[] = {
	{ DBUS_IFACE_ROOT,      root_methods,      root_properties      },
	{ DBUS_IFACE_PLAYER,    player_methods,    player_properties    },
	{ DBUS_IFACE_TRACKLIST, tracklist_methods, tracklist_properties },
	{ NULL,                 NULL,              NULL                 }
};

/*
 * Signal handlers & callbacks
 */

static void
on_player_notify(OckPlayer           *player,
                 GParamSpec          *pspec,
                 OckDbusServerMpris2 *self)
{
	OckDbusServer *dbus_server = OCK_DBUS_SERVER(self);
	const gchar *property_name = g_param_spec_get_name(pspec);
	const gchar *prop_name;
	GVariant *value;

	if (!g_strcmp0(property_name, "state")) {
		OckPlayerState state;

		state = ock_player_get_state(player);

		switch (state) {
		case OCK_PLAYER_STATE_PLAYING:
		case OCK_PLAYER_STATE_STOPPED:
			prop_name = "PlaybackStatus";
			value = g_variant_new_playback_status(player);
			break;
		default:
			/* Ignore other states */
			return;
		}
	} else if (!g_strcmp0(property_name, "repeat")) {
		prop_name = "LoopStatus";
		value = g_variant_new_loop_status(player);
	} else if (!g_strcmp0(property_name, "shuffle")) {
		prop_name = "Shuffle";
		value = g_variant_new_shuffle(player);
	} else if (!g_strcmp0(property_name, "volume")) {
		prop_name = "Volume";
		value = g_variant_new_volume(player);
	} else if (!g_strcmp0(property_name, "station") ||
	           !g_strcmp0(property_name, "metadata")) {
		OckStation *station;
		OckMetadata *metadata;

		station = ock_player_get_station(player);
		metadata = ock_player_get_metadata(player);

		prop_name = "Metadata";
		value = g_variant_new_metadata(station, metadata);
	} else {
		return;
	}

	ock_dbus_server_emit_signal_property_changed
	(dbus_server, DBUS_IFACE_PLAYER, prop_name, value);
}

static void
on_station_list_station_added(OckStationList      *station_list,
                              OckStation          *station,
                              OckDbusServerMpris2 *self)
{
	OckDbusServer *dbus_server = OCK_DBUS_SERVER(self);
	GVariantBuilder b;
	OckStation *after_station;
	gchar *after_track_id;

	after_station = ock_station_list_prev(station_list, station, FALSE, FALSE);
	after_track_id = make_track_id(after_station);

	g_variant_builder_init(&b, G_VARIANT_TYPE("(a{sv}o)"));
	g_variant_builder_add_value(&b, g_variant_new_metadata(station, NULL));
	g_variant_builder_add(&b, "o", after_track_id);

	ock_dbus_server_emit_signal(dbus_server, DBUS_IFACE_TRACKLIST, "TrackAdded",
	                            g_variant_builder_end(&b));

	g_free(after_track_id);
}

static void
on_station_list_station_removed(OckStationList      *station_list G_GNUC_UNUSED,
                                OckStation          *station,
                                OckDbusServerMpris2 *self)
{
	OckDbusServer *dbus_server = OCK_DBUS_SERVER(self);
	GVariantBuilder b;
	gchar *track_id;

	track_id = make_track_id(station);

	g_variant_builder_init(&b, G_VARIANT_TYPE("(o)"));
	g_variant_builder_add(&b, "o", track_id);

	ock_dbus_server_emit_signal(dbus_server, DBUS_IFACE_TRACKLIST, "TrackRemoved",
	                            g_variant_builder_end(&b));

	g_free(track_id);
}

static void
on_station_list_station_modified(OckStationList      *station_list G_GNUC_UNUSED,
                                 OckStation          *station,
                                 OckDbusServerMpris2 *self)
{
	OckDbusServer *dbus_server = OCK_DBUS_SERVER(self);
	GVariantBuilder b;
	gchar *track_id;

	track_id = make_track_id(station);

	g_variant_builder_init(&b, G_VARIANT_TYPE("(oa{sv})"));
	g_variant_builder_add(&b, "o", track_id);
	g_variant_builder_add_value(&b, g_variant_new_metadata(station, NULL));

	ock_dbus_server_emit_signal(dbus_server, DBUS_IFACE_TRACKLIST, "TrackMetadataChanged",
	                            g_variant_builder_end(&b));

	g_free(track_id);
}

/*
 * OckFeature methods
 */

static void
ock_dbus_server_mpris2_disable(OckFeature *feature)
{
	OckPlayer *player = ock_core_player;
	OckStationList *station_list = ock_core_station_list;

	/* Signal handlers */
	g_signal_handlers_disconnect_by_data(station_list, feature);
	g_signal_handlers_disconnect_by_data(player, feature);

	/* Chain up */
	OCK_FEATURE_CHAINUP_DISABLE(ock_dbus_server_mpris2, feature);
}

static void
ock_dbus_server_mpris2_enable(OckFeature *feature)
{
	OckPlayer *player = ock_core_player;
	OckStationList *station_list = ock_core_station_list;

	/* Chain up */
	OCK_FEATURE_CHAINUP_ENABLE(ock_dbus_server_mpris2, feature);

	/* Signal handlers */
	g_signal_connect(player, "notify", G_CALLBACK(on_player_notify), feature);
	g_signal_connect(station_list, "station-added",
	                 G_CALLBACK(on_station_list_station_added), feature);
	g_signal_connect(station_list, "station-removed",
	                 G_CALLBACK(on_station_list_station_removed), feature);
	g_signal_connect(station_list, "station-modified",
	                 G_CALLBACK(on_station_list_station_modified), feature);
}

/*
 * GObject methods
 */

static void
ock_dbus_server_mpris2_constructed(GObject *object)
{
	OckDbusServer *dbus_server = OCK_DBUS_SERVER(object);

	TRACE("%p", object);

	/* Set dbus server properties */
	ock_dbus_server_set_dbus_name(dbus_server, DBUS_NAME);
	ock_dbus_server_set_dbus_path(dbus_server, DBUS_PATH);
	ock_dbus_server_set_dbus_introspection(dbus_server, DBUS_INTROSPECTION);
	ock_dbus_server_set_dbus_interface_table(dbus_server, dbus_interfaces);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_dbus_server_mpris2, object);
}

static void
ock_dbus_server_mpris2_init(OckDbusServerMpris2 *self)
{
	TRACE("%p", self);
}

static void
ock_dbus_server_mpris2_class_init(OckDbusServerMpris2Class *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	OckFeatureClass *feature_class = OCK_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->constructed = ock_dbus_server_mpris2_constructed;

	/* Override OckFeature methods */
	feature_class->enable = ock_dbus_server_mpris2_enable;
	feature_class->disable = ock_dbus_server_mpris2_disable;
}
