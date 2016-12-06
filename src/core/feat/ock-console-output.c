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

#include <stdio.h>
#include <unistd.h>

#include <glib.h>
#include <glib-object.h>

#include "additions/glib.h"

#include "framework/log.h"
#include "framework/vt-codes.h"
#include "framework/ock-framework.h"

#include "core/ock-core.h"

#include "core/feat/ock-console-output.h"

/*
 * GObject definitions
 */

struct _OckConsoleOutput {
	/* Parent instance structure */
	OckFeature parent_instance;
};

G_DEFINE_TYPE(OckConsoleOutput, ock_console_output, OCK_TYPE_FEATURE)

/*
 * Helpers
 */

#define PRINT(fmt, ...)    g_print(fmt "\n", ##__VA_ARGS__)

static const gchar *
time_now(void)
{
	static gchar *now_str;
	GDateTime *now;

	g_free(now_str);

	now = g_date_time_new_now_local();
	now_str = g_date_time_format(now, "%T");
	g_date_time_unref(now);

	return now_str;
}

static void
print_hello_line(void)
{
	PRINT("---- " PACKAGE_CAMEL_NAME " " PACKAGE_VERSION " ----");
	PRINT("Hit Ctrl+C to quit...");
}

static void
print_goodbye_line(void)
{
	PRINT("---- Bye ----");
}

static void
print_error(const gchar *error_string)
{
	PRINT(VT_BOLD("Error !") " %s", error_string);
}

static void
print_station(OckStation *station)
{
	const gchar *str;

	if (station == NULL)
		return;

	str = ock_station_get_name(station);
	if (str) {
		PRINT(VT_BOLD("> %s Playing %s"), time_now(), str);
	} else {
		str = ock_station_get_uri(station);
		PRINT(VT_BOLD("> %s Playing <%s>"), time_now(), str);
	}
}

static void
print_metadata(OckMetadata *metadata)
{
	const gchar *artist;
	const gchar *title;
	const gchar *album;
	const gchar *year;
	const gchar *genre;

	if (metadata == NULL)
		return;

	artist = ock_metadata_get_artist(metadata);
	title = ock_metadata_get_title(metadata);
	album = ock_metadata_get_album(metadata);
	year = ock_metadata_get_year(metadata);
	genre = ock_metadata_get_genre(metadata);

	/* Ensure this first line is printed, with a timestamp */
	if (title == NULL)
		title = "(Unknown title)";

	PRINT(". %s %s", time_now(), title);

	/* Other fields are optional */
	if (artist)
		PRINT("           %s", artist);

	if (album && year)
		PRINT("           %s (%s)", album, year);
	else if (album)
		PRINT("           %s", album);
	else if (year)
		PRINT("           (%s)", year);

	if (genre)
		PRINT("           %s", genre);
}

/*
 * Signal handlers
 */

static void
on_player_notify(OckPlayer        *player,
                 GParamSpec       *pspec,
                 OckConsoleOutput *self G_GNUC_UNUSED)
{
	const gchar *property_name = g_param_spec_get_name(pspec);

	if (!g_strcmp0(property_name, "state")) {
		OckPlayerState state;

		state = ock_player_get_state(player);

		if (state == OCK_PLAYER_STATE_PLAYING) {
			OckStation *station;

			station = ock_player_get_station(player);
			print_station(station);
		}
	} else if (!g_strcmp0(property_name, "metadata")) {
		OckMetadata *metadata;

		metadata = ock_player_get_metadata(player);
		print_metadata(metadata);
	}
}

static void
on_errorable_error(OckErrorable *errorable G_GNUC_UNUSED, const gchar *error_string,
                   OckConsoleOutput *self G_GNUC_UNUSED)
{
	print_error(error_string);
}

/*
 * OckFeature methods
 */

static void
ock_console_output_disable(OckFeature *feature)
{
	OckPlayer *player = ock_core_player;

	/* Signal handlers */
	g_signal_handlers_disconnect_list_by_data(ock_framework_errorable_list, feature);
	g_signal_handlers_disconnect_by_data(player, feature);

	/* Say good-bye */
	print_goodbye_line();

	/* Chain up */
	OCK_FEATURE_CHAINUP_DISABLE(ock_console_output, feature);
}

static void
ock_console_output_enable(OckFeature *feature)
{
	OckPlayer *player = ock_core_player;

	/* Chain up */
	OCK_FEATURE_CHAINUP_ENABLE(ock_console_output, feature);

	/* Say hello */
	print_hello_line();

	/* Signal handlers */
	g_signal_connect(player, "notify", G_CALLBACK(on_player_notify), feature);
	g_signal_connect_list(ock_framework_errorable_list, "error",
	                      G_CALLBACK(on_errorable_error), feature);
}

/*
 * GObject methods
 */

static void
ock_console_output_init(OckConsoleOutput *self)
{
	TRACE("%p", self);
}

static void
ock_console_output_class_init(OckConsoleOutputClass *class)
{
	OckFeatureClass *feature_class = OCK_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override OckFeature methods */
	feature_class->enable = ock_console_output_enable;
	feature_class->disable = ock_console_output_disable;
}
