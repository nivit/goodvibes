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

#include "framework/log.h"
#include "framework/ock-framework.h"

#include "core/ock-conf.h"
#include "core/ock-engine.h"
#include "core/ock-player.h"
#include "core/ock-station-list.h"

#ifdef CONSOLE_OUTPUT_ENABLED
#include "core/feat/ock-console-output.h"
#endif
#ifdef DBUS_SERVER_NATIVE_ENABLED
#include "core/feat/ock-dbus-server-native.h"
#endif
#ifdef DBUS_SERVER_MPRIS2_ENABLED
#include "core/feat/ock-dbus-server-mpris2.h"
#endif
#ifdef INHIBIT_ENABLED
#include "core/feat/ock-inhibitor.h"
#endif

OckConf        *ock_core_conf;
OckStationList *ock_core_station_list;
OckPlayer      *ock_core_player;

static OckEngine  *ock_core_engine;
static OckFeature *features[8];

static gboolean
when_idle_finish_init(gpointer user_data)
{
	const gchar *string_to_play = user_data;

	ock_player_go(ock_core_player, string_to_play);

	return G_SOURCE_REMOVE;
}

void
ock_core_cool_down(void)
{
	OckConf *conf = ock_core_conf;

	/* Stop watching for changes in objects */
	ock_conf_unwatch(conf);
}

void
ock_core_warm_up(const gchar *string_to_play)
{
	OckConf        *conf         = ock_core_conf;
	OckStationList *station_list = ock_core_station_list;

	/* Load data files */
	ock_conf_load(conf);
	ock_station_list_load(station_list);

	/* Apply configuration to objects */
	ock_conf_apply(conf);

	/* Watch for changes in objects */
	ock_conf_watch(conf);

	/* Schedule a callback to play some music.
	 * DO NOT start playing now ! It's too early !
	 * There's some init code pending, that will be run
	 * only after the main loop is started.
	 */
	g_idle_add(when_idle_finish_init, (void *) string_to_play);
}

void
ock_core_cleanup(void)
{
	/* Features */

	OckFeature *feature;
	guint idx;

	for (idx = 0, feature = features[0]; feature != NULL; feature = features[++idx]) {
		ock_framework_features_remove(feature);
		ock_framework_configurables_remove(feature);
		ock_framework_errorables_remove(feature);
		g_object_unref(feature);
	}

	/* Core objects */

	OckConf        *conf         = ock_core_conf;
	OckEngine      *engine       = ock_core_engine;
	OckPlayer      *player       = ock_core_player;
	OckStationList *station_list = ock_core_station_list;

	ock_framework_configurables_remove(player);
	g_object_unref(player);

	g_object_unref(station_list);

	ock_framework_errorables_remove(engine);
	g_object_unref(engine);

	g_object_unref(conf);
}

void
ock_core_init(void)
{
	/* ----------------------------------------------- *
	 * Core objects                                    *
	 * ----------------------------------------------- */

	OckConf        *conf;
	OckEngine      *engine;
	OckPlayer      *player;
	OckStationList *station_list;

	conf = ock_conf_new();

	engine = ock_engine_new();
	ock_framework_errorables_append(engine);

	station_list = ock_station_list_new();

	player = ock_player_new(engine, station_list);
	ock_framework_configurables_append(player);



	/* ----------------------------------------------- *
	 * Features                                        *
	 * ----------------------------------------------- */

	OckFeature *feature;
	guint idx;

	/* Create every feature enabled at compile-time.
	 * We can't do that in a more generic way, since objects
	 * types are not constant and can't be put in an array.
	 */
	idx = 0;

#ifdef CONSOLE_OUTPUT_ENABLED
	feature = ock_feature_new(OCK_TYPE_CONSOLE_OUTPUT, FALSE);
	features[idx++] = feature;
	ock_framework_features_append(feature);
	ock_framework_configurables_append(feature);
#endif
#ifdef DBUS_SERVER_NATIVE_ENABLED
	feature = ock_feature_new(OCK_TYPE_DBUS_SERVER_NATIVE, TRUE);
	features[idx++] = feature;
	ock_framework_features_append(feature);
	ock_framework_configurables_append(feature);
#endif
#ifdef DBUS_SERVER_MPRIS2_ENABLED
	feature = ock_feature_new(OCK_TYPE_DBUS_SERVER_MPRIS2, TRUE);
	features[idx++] = feature;
	ock_framework_features_append(feature);
	ock_framework_configurables_append(feature);
#endif
#ifdef INHIBIT_ENABLED
	feature = ock_feature_new(OCK_TYPE_INHIBITOR, FALSE);
	features[idx++] = feature;
	ock_framework_features_append(feature);
	ock_framework_configurables_append(feature);
#endif



	/* ----------------------------------------------- *
	 * Initialize global variables                     *
	 * ----------------------------------------------- */

	ock_core_conf = conf;
	ock_core_engine = engine;
	ock_core_station_list = station_list;
	ock_core_player = player;
}

/*
 * Underlying audio backend
 */

#include <gst/gst.h>

#include "additions/gst.h"

void
ock_core_audio_backend_cleanup(void)
{
	if (gst_is_initialized())
		gst_deinit();
}

GOptionGroup *
ock_core_audio_backend_init_get_option_group(void)
{
	GOptionGroup *group;

	/* This call also runs the gst_init() code. It's not obvious when reading
	 * the documentation, but the name of the function strongly suggests it.
	 * Peeping into the code leaves no doubt though.
	 */
	group = gst_init_get_option_group();

	return group;
}

const gchar *
ock_core_audio_backend_runtime_version_string(void)
{
	return gst_get_runtime_version_string();
}

const gchar *
ock_core_audio_backend_compile_version_string(void)
{
	return gst_get_compile_version_string();
}
