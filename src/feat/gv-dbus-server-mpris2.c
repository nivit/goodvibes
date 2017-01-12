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

#include <string.h>
#include <math.h>

#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>

#include "additions/glib.h"
#include "additions/glib-object.h"

#include "framework/gv-framework.h"

#ifdef UI_ENABLED
#include "ui/gv-ui.h"
#endif

#include "core/gv-core.h"

#include "feat/gv-dbus-server.h"
#include "feat/gv-dbus-server-mpris2.h"

#define TRACKID_PATH         "/org/" PACKAGE_CAMEL_NAME "/StationList"
#define PLAYLISTID_PATH      "/org/" PACKAGE_CAMEL_NAME "/Playlist"

#define DBUS_NAME            "org.mpris.MediaPlayer2." PACKAGE_CAMEL_NAME
#define DBUS_PATH            "/org/mpris/MediaPlayer2"
#define DBUS_IFACE_ROOT      "org.mpris.MediaPlayer2"
#define DBUS_IFACE_PLAYER    DBUS_IFACE_ROOT ".Player"
#define DBUS_IFACE_TRACKLIST DBUS_IFACE_ROOT ".TrackList"
#define DBUS_IFACE_PLAYLISTS DBUS_IFACE_ROOT ".Playlists"

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
        "    <interface name='"DBUS_IFACE_PLAYLISTS"'>"
        "        <method name='ActivatePlaylist'>"
        "            <arg direction='in'  name='PlaylistId'   type='o'/>"
        "        </method>"
        "        <method name='GetPlaylists'>"
        "            <arg direction='in'  name='Index'        type='u'/>"
        "            <arg direction='in'  name='MaxCount'     type='u'/>"
        "            <arg direction='in'  name='Order'        type='s'/>"
        "            <arg direction='in'  name='ReverseOrder' type='b'/>"
        "            <arg direction='out' name='Playlists'    type='a(oss)'/>"
        "        </method>"
        "        <signal name='PlaylistChanged'>"
        "            <arg name='Playlist' type='(oss)'/>"
        "        </signal>"
        "        <property name='PlaylistCount'  type='u'        access='read'/>"
        "        <property name='Orderings'      type='as'       access='read'/>"
        "        <property name='ActivePlaylist' type='(b(oss))' access='read'/>"
        "    </interface>"
        "</node>";

/*
 * GObject definitions
 */

struct _GvDbusServerMpris2 {
	/* Parent instance structure */
	GvDbusServer parent_instance;
};

G_DEFINE_TYPE(GvDbusServerMpris2, gv_dbus_server_mpris2, GV_TYPE_DBUS_SERVER)

/*
 * Helpers
 */

static gchar *
make_playlist_id(GvStation *station)
{
	/* As suggested in the MPRIS2 specifications, "/" should be used if NULL.
	 * https://specifications.freedesktop.org/mpris-spec/latest/
	 * Playlists_Interface.html#Struct:Maybe_Playlist
	 */
	if (station == NULL)
		return g_strdup("/");

	return g_strdup_printf(PLAYLISTID_PATH "/%s", gv_station_get_uid(station));
}

static gboolean
parse_playlist_id(const gchar    *playlist_id,
                  GvStationList  *station_list,
                  GvStation     **station)
{
	const gchar *station_uid;

	g_assert_nonnull(station);
	*station = NULL;

	if (!g_strcmp0(playlist_id, "/"))
		return TRUE;

	if (!g_str_has_prefix(playlist_id, PLAYLISTID_PATH "/"))
		return FALSE;

	station_uid = playlist_id + strlen(PLAYLISTID_PATH "/");
	*station = gv_station_list_find_by_uid(station_list, station_uid);
	if (*station == NULL)
		return FALSE;

	return TRUE;
}

static gchar *
make_track_id(GvStation *station)
{
	if (station == NULL)
		return g_strdup(DBUS_PATH "/TrackList/NoTrack");

	return g_strdup_printf(TRACKID_PATH "/%s", gv_station_get_uid(station));
}

static gboolean
parse_track_id(const gchar     *track_id,
               GvStationList  *station_list,
               GvStation     **station)
{
	const gchar *station_uid;

	g_assert_nonnull(station);
	*station = NULL;

	if (!g_strcmp0(track_id, DBUS_PATH "/TrackList/NoTrack"))
		return TRUE;

	if (!g_str_has_prefix(track_id, TRACKID_PATH "/"))
		return FALSE;

	station_uid = track_id + strlen(TRACKID_PATH "/");
	*station = gv_station_list_find_by_uid(station_list, station_uid);
	if (*station == NULL)
		return FALSE;

	return TRUE;
}

static gint
compare_alphabetically(GvStation *a, GvStation *b)
{
	const gchar *str1 = gv_station_get_name_or_uri(a);
	const gchar *str2 = gv_station_get_name_or_uri(b);

	return g_strcmp0(str1, str2);
}

static GList *
build_station_list(GvStationList *station_list, gboolean alphabetical,
                   gboolean reverse, guint start_index, guint max_count)
{
	GvStationListIter *iter;
	GvStation *station;
	GList *list = NULL;

	/* Build a GList */
	iter = gv_station_list_iter_new(station_list);

	while (gv_station_list_iter_loop(iter, &station))
		list = g_list_append(list, station);

	gv_station_list_iter_free(iter);

	/* Order alphabetically if needed */
	if (alphabetical)
		list = g_list_sort(list, (GCompareFunc) compare_alphabetically);

	/* Reverse order if needed */
	if (reverse)
		list = g_list_reverse(list);

	/* Handle start index */
	while (start_index > 0) {
		if (list == NULL)
			break;

		list = g_list_delete_link(list, list);
		start_index--;
	}

	/* Handle max count */
	GList *tmp = list;

	while (max_count > 0) {
		if (tmp == NULL)
			break;

		tmp = tmp->next;
		max_count--;
	}

	if (tmp != NULL) {
		if (tmp == list) {
			g_list_free(list);
			list = NULL;
		} else {
			tmp->prev->next = NULL;
			tmp->prev = NULL;
			g_list_free(tmp);
		}
	}

	return list;
}

/*
 * GVariant helpers for MPRIS2 types
 */

static GVariant *
g_variant_new_playlist(GvStation *station)
{
	gchar *playlist_id;

	playlist_id = make_playlist_id(station);

	GVariant *tuples[] = {
		g_variant_new_object_path(playlist_id),
		g_variant_new_string(station ? gv_station_get_name_or_uri(station) : ""),
		g_variant_new_string("")
	};

	g_free(playlist_id);

	return g_variant_new_tuple(tuples, 3);
}

static GVariant *
g_variant_new_maybe_playlist(GvStation *station)
{
	GVariant *tuples[] = {
		g_variant_new_boolean(station ? TRUE : FALSE),
		g_variant_new_playlist(station)
	};

	return g_variant_new_tuple(tuples, 2);
}

static GVariant *
g_variant_new_metadata_map(GvStation *station, GvMetadata *metadata)
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

	uri = gv_station_get_uri(station);
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
g_variant_new_playback_status(GvPlayer *player)
{
	GvPlayerState state;
	gchar *state_str;

	state = gv_player_get_state(player);

	switch (state) {
	case GV_PLAYER_STATE_PLAYING:
		state_str = "Playing";
		break;
	case GV_PLAYER_STATE_STOPPED:
	default:
		state_str = "Stopped";
		break;
	}

	return g_variant_new_string(state_str);
}

static GVariant *
g_variant_new_loop_status(GvPlayer *player)
{
	gboolean repeat;

	repeat = gv_player_get_repeat(player);
	return g_variant_new_string(repeat ? "Playlist" : "None");
}

static GVariant *
g_variant_new_shuffle(GvPlayer *player)
{
	gboolean shuffle;

	shuffle = gv_player_get_shuffle(player);
	return g_variant_new_boolean(shuffle);
}

static GVariant *
g_variant_new_volume(GvPlayer *player)
{
	guint volume;

	volume = gv_player_get_volume(player);
	return g_variant_new_double((gdouble) volume / 100.0);
}

/*
 * Dbus method handlers
 */

static GVariant *
method_raise(GvDbusServer  *dbus_server G_GNUC_UNUSED,
             GVariant       *params G_GNUC_UNUSED,
             GError        **error G_GNUC_UNUSED)
{
#ifdef UI_ENABLED
	gv_ui_present_main();
#endif
	return NULL;
}

static GVariant *
method_quit(GvDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	gv_core_quit();

	return NULL;
}

static GvDbusMethod root_methods[] = {
	{ "Raise", method_raise },
	{ "Quit",  method_quit  },
	{ NULL,    NULL         }
};

static GVariant *
method_play(GvDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	gv_player_play(player);

	return NULL;
}

static GVariant *
method_stop(GvDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	gv_player_stop(player);

	return NULL;
}

static GVariant *
method_toggle(GvDbusServer  *dbus_server G_GNUC_UNUSED,
              GVariant       *params G_GNUC_UNUSED,
              GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	gv_player_toggle(player);

	return NULL;
}

static GVariant *
method_next(GvDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	if (!gv_player_next(player))
		gv_player_stop(player);

	return NULL;
}

static GVariant *
method_prev(GvDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	if (!gv_player_prev(player))
		gv_player_stop(player);

	return NULL;
}

static GVariant *
method_open_uri(GvDbusServer  *dbus_server G_GNUC_UNUSED,
                GVariant       *params G_GNUC_UNUSED,
                GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player  = gv_core_player;
	GvStationList *station_list = gv_core_station_list;
	GvStation *station;
	const gchar *uri;

	g_variant_get(params, "(&s)", &uri);

	/* Ensure URI is valid */
	if (!is_uri_scheme_supported(uri)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "URI scheme not supported.");
		return NULL;
	}

	/* Check if the station is part of the station list */
	station = gv_station_list_find_by_uri(station_list, uri);

	/* Create a new station if needed */
	if (station == NULL) {
		station = gv_station_new(NULL, uri);
		gv_station_list_append(station_list, station);
		g_object_unref(station);
	}

	/* Play station */
	gv_player_set_station(player, station);
	gv_player_play(player);

	return NULL;
}

static GvDbusMethod player_methods[] = {
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
method_get_tracks_metadata(GvDbusServer  *dbus_server G_GNUC_UNUSED,
                           GVariant       *params,
                           GError        **error G_GNUC_UNUSED)
{
	GvStationList *station_list = gv_core_station_list;
	GVariantBuilder b;
	GVariantIter *iter;
	const gchar *track_id;

	g_variant_get(params, "(ao)", &iter);
	g_variant_builder_init(&b, G_VARIANT_TYPE("aa{sv}"));
	while (g_variant_iter_loop(iter, "&o", &track_id)) {
		GvStation *station;

		if (!parse_track_id(track_id, station_list, &station))
			/* Ignore silently */
			continue;

		g_variant_builder_add_value(&b, g_variant_new_metadata_map(station, NULL));
	}

	return g_variant_builder_end(&b);
}

static GVariant *
method_add_track(GvDbusServer  *dbus_server G_GNUC_UNUSED,
                 GVariant       *params,
                 GError        **error)
{
	GvPlayer *player = gv_core_player;
	GvStationList *station_list = gv_core_station_list;
	const gchar *uri;
	const gchar *after_track_id;
	gboolean set_as_current;
	GvStation *station, *after_station;

	g_variant_get(params, "(&s&ob)", &uri, &after_track_id, &set_as_current);

	/* Ensure URI is valid */
	if (!is_uri_scheme_supported(uri)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid URI scheme for param 'Uri'.");
		return NULL;
	}

	/* Handle after track */
	if (!parse_track_id(after_track_id, station_list, &after_station)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid param 'AfterTrack'.");
		return NULL;
	}

	/* Add a new station to station list. If after_station is NULL,
	 * MPRIS2 indicates that the track should be placed at the
	 * beginning of the track list.
	 */
	station = gv_station_new(NULL, uri);

	if (after_station)
		gv_station_list_insert_after(station_list, station, after_station);
	else
		gv_station_list_prepend(station_list, station);

	g_object_unref(station);

	/* Play new station if needed */
	if (set_as_current) {
		gv_player_set_station(player, station);
		if (gv_player_get_state(player) != GV_PLAYER_STATE_STOPPED)
			gv_player_play(player);
	}

	return NULL;
}

static GVariant *
method_remove_track(GvDbusServer  *dbus_server G_GNUC_UNUSED,
                    GVariant       *params,
                    GError        **error)
{
	GvStationList *station_list = gv_core_station_list;
	const gchar *track_id;
	GvStation *station;

	g_variant_get(params, "(&o)", &track_id);
	if (!parse_track_id(track_id, station_list, &station)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid param 'TrackId'.");
		return NULL;
	}

	gv_station_list_remove(station_list, station);

	return NULL;
}

static GVariant *
method_go_to(GvDbusServer  *dbus_server G_GNUC_UNUSED,
             GVariant       *params,
             GError        **error)
{
	GvPlayer *player = gv_core_player;
	GvStationList *station_list = gv_core_station_list;
	const gchar *track_id;
	GvStation *station;

	// WISHED What about the last line in MPRIS2 specs ? What does that mean ?

	g_variant_get(params, "(&o)", &track_id);
	if (!parse_track_id(track_id, station_list, &station)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid param 'TrackId'.");
		return NULL;
	}

	gv_player_set_station(player, station);

	if (gv_player_get_state(player) != GV_PLAYER_STATE_STOPPED)
		gv_player_play(player);

	return NULL;
}

static GvDbusMethod tracklist_methods[] = {
	{ "GetTracksMetadata", method_get_tracks_metadata },
	{ "AddTrack",          method_add_track           },
	{ "RemoveTrack",       method_remove_track        },
	{ "GoTo",              method_go_to               },
	{ NULL,                NULL                       }
};

static GVariant *
method_activate_playlist(GvDbusServer  *dbus_server G_GNUC_UNUSED,
                         GVariant      *params,
                         GError       **error)
{
	GvPlayer *player = gv_core_player;
	GvStationList *station_list = gv_core_station_list;
	const gchar *playlist_id;
	GvStation *station;

	g_variant_get(params, "(&o)", &playlist_id);

	if (!parse_playlist_id(playlist_id, station_list, &station)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid param 'PlaylistId'.");
		return NULL;
	}

	if (station == NULL)
		return NULL;

	gv_player_set_station(player, station);
	gv_player_play(player);

	return NULL;
}

static GVariant *
method_get_playlists(GvDbusServer  *dbus_server G_GNUC_UNUSED,
                     GVariant      *params,
                     GError       **error G_GNUC_UNUSED)
{
	GvStationList *station_list = gv_core_station_list;
	GVariantBuilder b;
	GList *list, *item;
	guint32 start_index, max_count;
	const gchar *order;
	gboolean reverse_order;
	gboolean alphabetical;

	g_variant_get(params, "(uu&sb)", &start_index, &max_count, &order, &reverse_order);

	/* We only support 'Alphabetical' */
	if (!g_strcmp0(order, "Alphabetical"))
		alphabetical = TRUE;
	else
		alphabetical = FALSE;

	/* Make a list */
	list = build_station_list(station_list, alphabetical, reverse_order,
	                          start_index, max_count);

	/* Make a GVariant */
	g_variant_builder_init(&b, G_VARIANT_TYPE("a(oss)"));

	for (item = list; item; item = item->next) {
		GvStation *station = item->data;
		g_variant_builder_add_value(&b, g_variant_new_playlist(station));
	}

	/* Cleanup */
	g_list_free(list);

	/* Return */
	return g_variant_builder_end(&b);
}

static GvDbusMethod playlists_methods[] = {
	{ "ActivatePlaylist",  method_activate_playlist },
	{ "GetPlaylists",      method_get_playlists     },
	{ NULL,                NULL                     }
};

/*
 * Dbus property handlers
 */

static GVariant *
prop_get_true(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	/* Dummy accessor to return TRUE all the time */
	return g_variant_new_boolean(TRUE);
}

static GVariant *
prop_get_false(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	/* Dummy accessor to return FALSE all the time */
	return g_variant_new_boolean(FALSE);
}

static gboolean
prop_set_error(GvDbusServer  *dbus_server G_GNUC_UNUSED,
               GVariant       *value G_GNUC_UNUSED,
               GError        **error)
{
	g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
	            "Setting this property is not supported.");

	return FALSE;
}

static GVariant *
prop_get_can_raise(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
#ifdef UI_ENABLED
	return g_variant_new_boolean(TRUE);
#else
	return g_variant_new_boolean(FALSE);
#endif
}

static GVariant *
prop_get_identity(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	// TODO Add gettext stuff for translating, I saw that somewhere.
	//      But, wait, what does the MPRIS2 spec say about that ?
	return g_variant_new_string(PACKAGE_LONG_NAME);
}

static GVariant *
prop_get_desktop_entry(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	return g_variant_new_string(PACKAGE_NAME);
}

static GVariant *
prop_get_supported_uri_schemes(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	return g_variant_new_strv(SUPPORTED_URI_SCHEMES, -1);
}

static GVariant *
prop_get_supported_mime_types(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	return g_variant_new_strv(SUPPORTED_MIME_TYPES, -1);
}

static GvDbusProperty root_properties[] = {
	{ "CanRaise",            prop_get_can_raise,             NULL },
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
prop_get_playback_status(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	return g_variant_new_playback_status(player);
}

static GVariant *
prop_get_loop_status(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	return g_variant_new_loop_status(player);
}

static gboolean
prop_set_loop_status(GvDbusServer  *dbus_server G_GNUC_UNUSED,
                     GVariant       *value,
                     GError        **error)
{
	GvPlayer *player = gv_core_player;
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

	gv_player_set_repeat(player, repeat);

	return TRUE;
}

static GVariant *
prop_get_shuffle(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	return g_variant_new_shuffle(player);
}

static gboolean
prop_set_shuffle(GvDbusServer  *dbus_server G_GNUC_UNUSED,
                 GVariant       *value,
                 GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	gboolean shuffle;

	shuffle = g_variant_get_boolean(value);
	gv_player_set_shuffle(player, shuffle);

	return TRUE;
}

static GVariant *
prop_get_volume(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	return g_variant_new_volume(player);
}

static gboolean
prop_set_volume(GvDbusServer  *dbus_server G_GNUC_UNUSED,
                GVariant       *value,
                GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	gdouble volume;

	volume = g_variant_get_double(value);
	volume = round(volume * 100);
	gv_player_set_volume(player, (guint) volume);

	return TRUE;
}

static GVariant *
prop_get_rate(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	return g_variant_new_double(1.0);
}

static GVariant *
prop_get_metadata(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	GvStation *station;
	GvMetadata *metadata;

	station = gv_player_get_station(player);
	metadata = gv_player_get_metadata(player);

	return g_variant_new_metadata_map(station, metadata);
}

static GVariant *
prop_get_can_play(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvStationList *station_list = gv_core_station_list;
	guint n_stations;

	n_stations = gv_station_list_get_length(station_list);

	return g_variant_new_boolean(n_stations > 0 ? TRUE : FALSE);
}

static GVariant *
prop_get_can_go_prev(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	gboolean has_prev;

	has_prev = gv_player_get_prev_station(player) ? TRUE : FALSE;

	return g_variant_new_boolean(has_prev);
}

static GVariant *
prop_get_can_go_next(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	gboolean has_next;

	has_next = gv_player_get_next_station(player) ? TRUE : FALSE;

	return g_variant_new_boolean(has_next);
}

static GvDbusProperty player_properties[] = {
	{ "PlaybackStatus", prop_get_playback_status, NULL },
	{ "LoopStatus",     prop_get_loop_status,     prop_set_loop_status },
	{ "Shuffle",        prop_get_shuffle,         prop_set_shuffle },
	{ "Volume",         prop_get_volume,          prop_set_volume },
	{ "Rate",           prop_get_rate,            prop_set_error },
	{ "MinimumRate",    prop_get_rate,            NULL },
	{ "MaximumRate",    prop_get_rate,            NULL },
	{ "Metadata",       prop_get_metadata,        NULL },
	{ "CanPlay",        prop_get_can_play,        NULL },
	{ "CanPause",       prop_get_can_play,        NULL },
	{ "CanGoNext",      prop_get_can_go_next,     NULL },
	{ "CanGoPrevious",  prop_get_can_go_prev,     NULL },
	{ "CanSeek",        prop_get_false,           NULL },
	{ "CanControl",     prop_get_true,            NULL },
	{ NULL,             NULL,                     NULL }
};

static GVariant *
prop_get_tracks(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvStationList *station_list = gv_core_station_list;
	GvStationListIter *iter;
	GvStation *station;
	GVariantBuilder b;

	g_variant_builder_init(&b, G_VARIANT_TYPE("ao"));
	iter = gv_station_list_iter_new(station_list);

	while (gv_station_list_iter_loop(iter, &station)) {
		gchar *track_id;
		track_id = make_track_id(station);
		g_variant_builder_add(&b, "o", track_id);
		g_free(track_id);
	}

	gv_station_list_iter_free(iter);
	return g_variant_builder_end(&b);
}

static GvDbusProperty tracklist_properties[] = {
	{ "Tracks",        prop_get_tracks, NULL },
	{ "CanEditTracks", prop_get_true,   NULL },
	{ NULL,            NULL,            NULL }
};

static GVariant *
prop_get_playlist_count(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvStationList *station_list = gv_core_station_list;
	guint n_stations = gv_station_list_get_length(station_list);

	return g_variant_new_uint32(n_stations);
}

static GVariant *
prop_get_orderings(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	static const gchar *supported_orderings[] = {
		"Alphabetical", "UserDefined", NULL
	};

	return g_variant_new_strv(supported_orderings, -1);
}

static GVariant *
prop_get_active_playlist(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	GvStation *station = gv_player_get_station(player);

	return g_variant_new_maybe_playlist(station);
}

static GvDbusProperty playlists_properties[] = {
	{ "PlaylistCount",  prop_get_playlist_count,  NULL },
	{ "Orderings",      prop_get_orderings,       NULL },
	{ "ActivePlaylist", prop_get_active_playlist, NULL },
	{ NULL,             NULL,                     NULL }
};

/*
 * Dbus interfaces
 */

static GvDbusInterface dbus_interfaces[] = {
	{ DBUS_IFACE_ROOT,      root_methods,      root_properties      },
	{ DBUS_IFACE_PLAYER,    player_methods,    player_properties    },
	{ DBUS_IFACE_TRACKLIST, tracklist_methods, tracklist_properties },
	{ DBUS_IFACE_PLAYLISTS, playlists_methods, playlists_properties },
	{ NULL,                 NULL,              NULL                 }
};

/*
 * Signal handlers & callbacks
 */

static void
on_player_notify(GvPlayer           *player,
                 GParamSpec          *pspec,
                 GvDbusServerMpris2 *self)
{
	GvDbusServer *dbus_server = GV_DBUS_SERVER(self);
	const gchar *property_name = g_param_spec_get_name(pspec);

	if (!g_strcmp0(property_name, "state")) {
		GvPlayerState state = gv_player_get_state(player);

		if (state != GV_PLAYER_STATE_PLAYING &&
		    state != GV_PLAYER_STATE_STOPPED)
			return;

		gv_dbus_server_emit_signal_property_changed
		(dbus_server, DBUS_IFACE_PLAYER, "PlaybackStatus",
		 g_variant_new_playback_status(player));

	} else if (!g_strcmp0(property_name, "repeat")) {
		gv_dbus_server_emit_signal_property_changed
		(dbus_server, DBUS_IFACE_PLAYER, "LoopStatus",
		 g_variant_new_loop_status(player));

	} else if (!g_strcmp0(property_name, "shuffle")) {
		gv_dbus_server_emit_signal_property_changed
		(dbus_server, DBUS_IFACE_PLAYER, "Shuffle",
		 g_variant_new_shuffle(player));

	} else if (!g_strcmp0(property_name, "volume")) {
		gv_dbus_server_emit_signal_property_changed
		(dbus_server, DBUS_IFACE_PLAYER, "Volume",
		 g_variant_new_volume(player));

	} else if (!g_strcmp0(property_name, "station")) {
		GvStation *station = gv_player_get_station(player);
		GvMetadata *metadata = gv_player_get_metadata(player);

		gv_dbus_server_emit_signal_property_changed
		(dbus_server, DBUS_IFACE_PLAYER, "Metadata",
		 g_variant_new_metadata_map(station, metadata));

		/* This signal should be send only if the station's name
		 * or the station's icon was changed.
		 */
		gv_dbus_server_emit_signal_property_changed
		(dbus_server, DBUS_IFACE_PLAYLISTS, "PlaylistChanged",
		 g_variant_new_playlist(station));

	} else if (!g_strcmp0(property_name, "metadata")) {
		GvStation *station = gv_player_get_station(player);
		GvMetadata *metadata = gv_player_get_metadata(player);

		gv_dbus_server_emit_signal_property_changed
		(dbus_server, DBUS_IFACE_PLAYER, "Metadata",
		 g_variant_new_metadata_map(station, metadata));
	}
}

static void
on_station_list_station_added(GvStationList      *station_list,
                              GvStation          *station,
                              GvDbusServerMpris2 *self)
{
	GvDbusServer *dbus_server = GV_DBUS_SERVER(self);
	GVariantBuilder b;
	GvStation *after_station;
	gchar *after_track_id;

	after_station = gv_station_list_prev(station_list, station, FALSE, FALSE);
	after_track_id = make_track_id(after_station);

	g_variant_builder_init(&b, G_VARIANT_TYPE("(a{sv}o)"));
	g_variant_builder_add_value(&b, g_variant_new_metadata_map(station, NULL));
	g_variant_builder_add(&b, "o", after_track_id);

	gv_dbus_server_emit_signal(dbus_server, DBUS_IFACE_TRACKLIST, "TrackAdded",
	                           g_variant_builder_end(&b));

	g_free(after_track_id);
}

static void
on_station_list_station_removed(GvStationList      *station_list G_GNUC_UNUSED,
                                GvStation          *station,
                                GvDbusServerMpris2 *self)
{
	GvDbusServer *dbus_server = GV_DBUS_SERVER(self);
	GVariantBuilder b;
	gchar *track_id;

	track_id = make_track_id(station);

	g_variant_builder_init(&b, G_VARIANT_TYPE("(o)"));
	g_variant_builder_add(&b, "o", track_id);

	gv_dbus_server_emit_signal(dbus_server, DBUS_IFACE_TRACKLIST, "TrackRemoved",
	                           g_variant_builder_end(&b));

	g_free(track_id);
}

static void
on_station_list_station_modified(GvStationList      *station_list G_GNUC_UNUSED,
                                 GvStation          *station,
                                 GvDbusServerMpris2 *self)
{
	GvDbusServer *dbus_server = GV_DBUS_SERVER(self);
	GVariantBuilder b;
	gchar *track_id;

	track_id = make_track_id(station);

	g_variant_builder_init(&b, G_VARIANT_TYPE("(oa{sv})"));
	g_variant_builder_add(&b, "o", track_id);
	g_variant_builder_add_value(&b, g_variant_new_metadata_map(station, NULL));

	gv_dbus_server_emit_signal(dbus_server, DBUS_IFACE_TRACKLIST, "TrackMetadataChanged",
	                           g_variant_builder_end(&b));

	g_free(track_id);
}

/*
 * GvFeature methods
 */

static void
gv_dbus_server_mpris2_disable(GvFeature *feature)
{
	GvPlayer *player = gv_core_player;
	GvStationList *station_list = gv_core_station_list;

	/* Signal handlers */
	g_signal_handlers_disconnect_by_data(station_list, feature);
	g_signal_handlers_disconnect_by_data(player, feature);

	/* Chain up */
	GV_FEATURE_CHAINUP_DISABLE(gv_dbus_server_mpris2, feature);
}

static void
gv_dbus_server_mpris2_enable(GvFeature *feature)
{
	GvPlayer *player = gv_core_player;
	GvStationList *station_list = gv_core_station_list;

	/* Chain up */
	GV_FEATURE_CHAINUP_ENABLE(gv_dbus_server_mpris2, feature);

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
gv_dbus_server_mpris2_constructed(GObject *object)
{
	GvDbusServer *dbus_server = GV_DBUS_SERVER(object);

	TRACE("%p", object);

	/* Set dbus server properties */
	gv_dbus_server_set_dbus_name(dbus_server, DBUS_NAME);
	gv_dbus_server_set_dbus_path(dbus_server, DBUS_PATH);
	gv_dbus_server_set_dbus_introspection(dbus_server, DBUS_INTROSPECTION);
	gv_dbus_server_set_dbus_interface_table(dbus_server, dbus_interfaces);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(gv_dbus_server_mpris2, object);
}

static void
gv_dbus_server_mpris2_init(GvDbusServerMpris2 *self)
{
	TRACE("%p", self);
}

static void
gv_dbus_server_mpris2_class_init(GvDbusServerMpris2Class *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	GvFeatureClass *feature_class = GV_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->constructed = gv_dbus_server_mpris2_constructed;

	/* Override GvFeature methods */
	feature_class->enable = gv_dbus_server_mpris2_enable;
	feature_class->disable = gv_dbus_server_mpris2_disable;
}
