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

#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>

#include "additions/glib.h"
#include "additions/glib-object.h"

#include "framework/gv-framework.h"

#include "core/gv-core.h"

#include "feat/gv-dbus-server.h"
#include "feat/gv-dbus-server-native.h"

#define DBUS_PATH           PACKAGE_APPLICATION_PATH
#define DBUS_IFACE_ROOT     PACKAGE_APPLICATION_ID
#define DBUS_IFACE_PLAYER   DBUS_IFACE_ROOT ".Player"
#define DBUS_IFACE_STATIONS DBUS_IFACE_ROOT ".Stations"

static const gchar *DBUS_INTROSPECTION =
        "<node>"
        "    <interface name='"DBUS_IFACE_ROOT"'>"
        "        <method name='Quit'/>"
        "        <property name='Version' type='s' access='read'/>"
        "    </interface>"
        "    <interface name='"DBUS_IFACE_PLAYER"'>"
        "        <method name='Play'>"
        "            <arg direction='in' name='Station' type='s'/>"
        "        </method>"
        "        <method name='Stop'/>"
        "        <method name='Next'/>"
        "        <method name='Previous'/>"
        "        <property name='Current' type='a{sv}' access='read'/>"
        "        <property name='Playing' type='b'     access='read'/>"
        "        <property name='Repeat'  type='b'     access='readwrite'/>"
        "        <property name='Shuffle' type='b'     access='readwrite'/>"
        "        <property name='Volume'  type='u'     access='readwrite'/>"
        "        <property name='Mute'    type='b'     access='readwrite'/>"
        "    </interface>"
        "    <interface name='"DBUS_IFACE_STATIONS"'>"
        "        <method name='List'>"
        "            <arg direction='out' name='Stations'      type='aa{sv}'/>"
        "        </method>"
        "        <method name='Add'>"
        "            <arg direction='in'  name='StationUri'    type='s'/>"
        "            <arg direction='in'  name='StationName'   type='s'/>"
        "            <arg direction='in'  name='Where'         type='s'/>"
        "            <arg direction='in'  name='AroundStation' type='s'/>"
        "        </method>"
        "        <method name='Remove'>"
        "            <arg direction='in'  name='Station'       type='s'/>"
        "        </method>"
        "        <method name='Rename'>"
        "            <arg direction='in'  name='Station'       type='s'/>"
        "            <arg direction='in'  name='Name'          type='s'/>"
        "        </method>"
        "        <method name='Move'>"
        "            <arg direction='in'  name='Station'       type='s'/>"
        "            <arg direction='in'  name='Where'         type='s'/>"
        "            <arg direction='in'  name='AroundStation' type='s'/>"
        "        </method>"
        "    </interface>"
        "</node>";

/*
 * GObject definitions
 */

struct _GvDbusServerNative {
	/* Parent instance structure */
	GvDbusServer parent_instance;
};

G_DEFINE_TYPE(GvDbusServerNative, gv_dbus_server_native, GV_TYPE_DBUS_SERVER)

/*
 * Helpers
 */

static GVariant *
g_variant_new_station(GvStation *station, GvMetadata *metadata)
{
	GVariantBuilder b;
	const gchar *uri;
	const gchar *name;
	const gchar *artist;
	const gchar *title;
	const gchar *album;
	const gchar *genre;
	const gchar *year;
	const gchar *comment;

	g_variant_builder_init(&b, G_VARIANT_TYPE("a{sv}"));

	/* Station properties */
	if (station == NULL)
		goto end;

	g_object_get(station,
	             "uri", &uri,
	             "name", &name,
	             NULL);

	if (uri)
		g_variant_builder_add_dictentry_string(&b, "uri", uri);
	if (name)
		g_variant_builder_add_dictentry_string(&b, "name", name);

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
		g_variant_builder_add_dictentry_string(&b, "artist", artist);
	if (title)
		g_variant_builder_add_dictentry_string(&b, "title", title);
	if (album)
		g_variant_builder_add_dictentry_string(&b, "album", album);
	if (genre)
		g_variant_builder_add_dictentry_string(&b, "genre", genre);
	if (year)
		g_variant_builder_add_dictentry_string(&b, "year", year);
	if (comment)
		g_variant_builder_add_dictentry_string(&b, "comment", comment);

end:
	return g_variant_builder_end(&b);
}

/*
 * Dbus method handlers
 */

static GVariant *
method_quit(GvDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	gv_core_quit();

	return NULL;
}

static GvDbusMethod root_methods[] = {
	{ "Quit", method_quit },
	{ NULL,   NULL        }
};

static GVariant *
method_play(GvDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params,
            GError        **error)
{
	GvPlayer *player = gv_core_player;
	gchar *string;

	g_variant_get(params, "(&s)", &string);

	/* Empty string: play current station */
	if (!g_strcmp0(string, "")) {
		gv_player_play(player);
		return NULL;
	}

	/* Otherwise, string may be a station URI, or a station name.
	 * It may be part of station list, or may be a new URI.
	 */
	if (gv_player_set_station_by_guessing(player, string)) {
		gv_player_play(player);
		return NULL;
	}

	if (is_uri_scheme_supported(string)) {
		GvStation *station;

		station = gv_station_new(NULL, string);
		gv_player_set_station(player, station);
		g_object_unref(station);
		gv_player_play(player);
		return NULL;
	}

	g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
	            "'%s' is neither a known station or a valid uri",
	            string);

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
method_next(GvDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error  G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	gv_player_next(player);

	return NULL;
}

static GVariant *
method_prev(GvDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	gv_player_prev(player);

	return NULL;
}

static GvDbusMethod player_methods[] = {
	{ "Play",     method_play },
	{ "Stop",     method_stop },
	{ "Next",     method_next },
	{ "Previous", method_prev },
	{ NULL,       NULL        }
};

static GVariant *
method_list(GvDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params G_GNUC_UNUSED,
            GError        **error G_GNUC_UNUSED)
{
	GvStationList *station_list = gv_core_station_list;
	GvStationListIter *iter;
	GvStation *station;
	GVariantBuilder b;

	g_variant_builder_init(&b, G_VARIANT_TYPE("aa{sv}"));
	iter = gv_station_list_iter_new(station_list);

	while (gv_station_list_iter_loop(iter, &station))
		g_variant_builder_add_value(&b, g_variant_new_station(station, NULL));

	gv_station_list_iter_free(iter);
	return g_variant_builder_end(&b);
}

static GVariant *
method_add(GvDbusServer  *dbus_server G_GNUC_UNUSED,
           GVariant       *params,
           GError        **error)
{
	GvStationList *station_list = gv_core_station_list;
	GvStation *new_station;
	GvStation *around_station;
	gchar *uri;
	gchar *name;
	gchar *where;
	gchar *around;

	g_variant_get(params, "(&s&s&s&s)", &uri, &name, &where, &around);

	/* Handle new station */
	if (!is_uri_scheme_supported(uri)) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "URI scheme not supported");
		return NULL;
	}

	new_station = gv_station_new(name, uri);

	/* Handle where to add */
	around_station = gv_station_list_find_by_guessing(station_list, around);
	if (!g_strcmp0(where, "first"))
		gv_station_list_prepend(station_list, new_station);
	else if (!g_strcmp0(where, "last") || !g_strcmp0(where, ""))
		gv_station_list_append(station_list, new_station);
	else if (!g_strcmp0(where, "before"))
		gv_station_list_insert_before(station_list, new_station, around_station);
	else if (!g_strcmp0(where, "after"))
		gv_station_list_insert_after(station_list, new_station, around_station);
	else
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid keyword '%s'", where);

	g_object_unref(new_station);

	return NULL;
}

static GVariant *
method_remove(GvDbusServer  *dbus_server G_GNUC_UNUSED,
              GVariant       *params,
              GError        **error)
{
	GvStationList *station_list = gv_core_station_list;
	GvStation *match;
	gchar *station;

	g_variant_get(params, "(&s)", &station);

	match = gv_station_list_find_by_guessing(station_list, station);
	if (match)
		gv_station_list_remove(station_list, match);
	else
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Station '%s' not found", station);

	return NULL;
}

static GVariant *
method_rename(GvDbusServer  *dbus_server G_GNUC_UNUSED,
              GVariant       *params,
              GError        **error)
{
	GvStationList *station_list = gv_core_station_list;
	GvStation *match;
	gchar *station;
	gchar *name;

	g_variant_get(params, "(&s&s)", &station, &name);

	match = gv_station_list_find_by_guessing(station_list, station);
	if (match)
		gv_station_set_name(match, name);
	else
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Station '%s' not found", station);

	return NULL;
}

static GVariant *
method_move(GvDbusServer  *dbus_server G_GNUC_UNUSED,
            GVariant       *params,
            GError        **error)
{
	GvStationList *station_list = gv_core_station_list;
	GvStation *moving_station;
	GvStation *around_station;
	gchar *moving;
	gchar *around;
	gchar *where;

	g_variant_get(params, "(&s&s&s)", &moving, &where, &around);

	/* Handle station to move */
	moving_station = gv_station_list_find_by_guessing(station_list, moving);
	if (moving_station == NULL) {
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Station '%s' not found", moving);
		return NULL;
	}

	/* Move the station */
	around_station = gv_station_list_find_by_guessing(station_list, around);
	if (!g_strcmp0(where, "first"))
		gv_station_list_move_first(station_list, moving_station);
	else if (!g_strcmp0(where, "last"))
		gv_station_list_move_last(station_list, moving_station);
	else if (!g_strcmp0(where, "before"))
		gv_station_list_move_before(station_list, moving_station, around_station);
	else if (!g_strcmp0(where, "after"))
		gv_station_list_move_after(station_list, moving_station, around_station);
	else
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
		            "Invalid keyword '%s'", where);

	return NULL;
}

static GvDbusMethod stations_methods[] = {
	{ "List",   method_list   },
	{ "Add",    method_add    },
	{ "Remove", method_remove },
	{ "Rename", method_rename },
	{ "Move",   method_move   },
	{ NULL,     NULL          }
};

/*
 * Dbus property handlers
 */

static GVariant *
prop_get_version(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	return g_variant_new_string(PACKAGE_VERSION);
}

static GvDbusProperty root_properties[] = {
	{ "Version", prop_get_version, NULL },
	{ NULL,      NULL,             NULL }
};

static GVariant *
prop_get_current(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	GvStation *station;
	GvMetadata *metadata;

	station = gv_player_get_station(player);
	metadata = gv_player_get_metadata(player);

	return g_variant_new_station(station, metadata);
}

static GVariant *
prop_get_playing(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	GvPlayerState player_state;
	gboolean is_playing;

	player_state = gv_player_get_state(player);
	is_playing = player_state == GV_PLAYER_STATE_PLAYING ? TRUE : FALSE;

	return g_variant_new_boolean(is_playing);
}

static GVariant *
prop_get_repeat(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	gboolean repeat;

	repeat = gv_player_get_repeat(player);

	return g_variant_new_boolean(repeat);
}

static gboolean
prop_set_repeat(GvDbusServer  *dbus_server G_GNUC_UNUSED,
                GVariant       *value,
                GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	gboolean repeat;

	repeat = g_variant_get_boolean(value);
	gv_player_set_repeat(player, repeat);

	return TRUE;
}

static GVariant *
prop_get_shuffle(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	gboolean shuffle;

	shuffle = gv_player_get_shuffle(player);

	return g_variant_new_boolean(shuffle);
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
	guint volume;

	volume = gv_player_get_volume(player);

	return g_variant_new_uint32(volume);
}

static gboolean
prop_set_volume(GvDbusServer  *dbus_server G_GNUC_UNUSED,
                GVariant       *value,
                GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	guint volume;

	volume = g_variant_get_uint32(value);
	gv_player_set_volume(player, volume);

	return TRUE;
}

static GVariant *
prop_get_mute(GvDbusServer *dbus_server G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	gboolean mute;

	mute = gv_player_get_mute(player);

	return g_variant_new_boolean(mute);
}

static gboolean
prop_set_mute(GvDbusServer  *dbus_server G_GNUC_UNUSED,
              GVariant       *value,
              GError        **error G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;
	gboolean mute;

	mute = g_variant_get_boolean(value);
	gv_player_set_mute(player, mute);

	return TRUE;
}

static GvDbusProperty player_properties[] = {
	{ "Current", prop_get_current, NULL             },
	{ "Playing", prop_get_playing, NULL             },
	{ "Repeat",  prop_get_repeat,  prop_set_repeat  },
	{ "Shuffle", prop_get_shuffle, prop_set_shuffle },
	{ "Volume",  prop_get_volume,  prop_set_volume  },
	{ "Mute",    prop_get_mute,    prop_set_mute    },
	{ NULL,      NULL,                        NULL  }
};

/*
 * Dbus interfaces
 */

static GvDbusInterface dbus_interfaces[] = {
	{ DBUS_IFACE_ROOT,     root_methods,      root_properties   },
	{ DBUS_IFACE_PLAYER,   player_methods,    player_properties },
	{ DBUS_IFACE_STATIONS, stations_methods,  NULL              },
	{ NULL,                NULL,              NULL              }
};

/*
 * GObject methods
 */

static void
gv_dbus_server_native_constructed(GObject *object)
{
	GvDbusServer *dbus_server = GV_DBUS_SERVER(object);

	/* Set dbus server properties - we don't set a name as we don't want to acquire any */
	gv_dbus_server_set_dbus_path(dbus_server, DBUS_PATH);
	gv_dbus_server_set_dbus_introspection(dbus_server, DBUS_INTROSPECTION);
	gv_dbus_server_set_dbus_interface_table(dbus_server, dbus_interfaces);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(gv_dbus_server_native, object);
}

static void
gv_dbus_server_native_init(GvDbusServerNative *self)
{
	TRACE("%p", self);
}

static void
gv_dbus_server_native_class_init(GvDbusServerNativeClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->constructed = gv_dbus_server_native_constructed;
}
