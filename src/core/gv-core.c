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

#include <glib.h>

#include "framework/log.h"
#include "framework/gv-framework.h"

#include "core/gv-conf.h"
#include "core/gv-engine.h"
#include "core/gv-player.h"
#include "core/gv-station-list.h"

#ifdef CONSOLE_OUTPUT_ENABLED
#include "core/feat/gv-console-output.h"
#endif
#ifdef DBUS_SERVER_NATIVE_ENABLED
#include "core/feat/gv-dbus-server-native.h"
#endif
#ifdef DBUS_SERVER_MPRIS2_ENABLED
#include "core/feat/gv-dbus-server-mpris2.h"
#endif
#ifdef INHIBITOR_ENABLED
#include "core/feat/gv-inhibitor.h"
#endif

GvConf        *gv_core_conf;
GvStationList *gv_core_station_list;
GvPlayer      *gv_core_player;

static GvEngine  *gv_core_engine;
static GvFeature *features[8];

static gboolean
when_idle_finish_init(gpointer user_data)
{
	const gchar *string_to_play = user_data;

	gv_player_go(gv_core_player, string_to_play);

	return G_SOURCE_REMOVE;
}

void
gv_core_cool_down(void)
{
	GvConf *conf = gv_core_conf;

	/* Stop watching for changes in objects */
	gv_conf_unwatch(conf);
}

void
gv_core_warm_up(const gchar *string_to_play)
{
	GvConf        *conf         = gv_core_conf;
	GvStationList *station_list = gv_core_station_list;

	/* Load data files */
	gv_conf_load(conf);
	gv_station_list_load(station_list);

	/* Apply configuration to objects */
	gv_conf_apply(conf);

	/* Watch for changes in objects */
	gv_conf_watch(conf);

	/* Schedule a callback to play some music.
	 * DO NOT start playing now ! It's too early !
	 * There's some init code pending, that will be run only after the main
	 * loop is started, and might even do some async call (dbus connection).
	 * As much as possible, we wish that we start playing music after
	 * everything is setup. Therefore, we schedule with a low priority.
	 * This will give maximum chances to the init code to finish before
	 * music starts playing.
	 */
	g_idle_add_full(G_PRIORITY_LOW, when_idle_finish_init,
	                (void *) string_to_play, NULL);
}

void
gv_core_cleanup(void)
{
	/* Features */

	GvFeature *feature;
	guint idx;

	for (idx = 0, feature = features[0]; feature != NULL; feature = features[++idx]) {
		gv_framework_features_remove(feature);
		gv_framework_configurables_remove(feature);
		gv_framework_errorables_remove(feature);
		g_object_unref(feature);
	}

	/* Core objects */

	GvConf        *conf         = gv_core_conf;
	GvEngine      *engine       = gv_core_engine;
	GvPlayer      *player       = gv_core_player;
	GvStationList *station_list = gv_core_station_list;

	gv_framework_configurables_remove(player);
	gv_framework_errorables_remove(player);
	g_object_unref(player);

	gv_framework_errorables_remove(station_list);
	g_object_unref(station_list);

	gv_framework_errorables_remove(engine);
	g_object_unref(engine);

	gv_framework_errorables_remove(conf);
	g_object_unref(conf);
}

void
gv_core_init(void)
{
	/* ----------------------------------------------- *
	 * Core objects                                    *
	 * ----------------------------------------------- */

	GvConf        *conf;
	GvEngine      *engine;
	GvPlayer      *player;
	GvStationList *station_list;

	conf = gv_conf_new();
	gv_framework_errorables_append(conf);

	engine = gv_engine_new();
	gv_framework_errorables_append(engine);

	station_list = gv_station_list_new();
	gv_framework_errorables_append(station_list);

	player = gv_player_new(engine, station_list);
	gv_framework_configurables_append(player);
	gv_framework_errorables_append(player);


	/* ----------------------------------------------- *
	 * Features                                        *
	 * ----------------------------------------------- */

	GvFeature *feature;
	guint idx;

	/* Create every feature enabled at compile-time.
	 * We can't do that in a more generic way, since objects
	 * types are not constant and can't be put in an array.
	 */
	idx = 0;

#ifdef CONSOLE_OUTPUT_ENABLED
	feature = gv_feature_new(GV_TYPE_CONSOLE_OUTPUT, FALSE);
	features[idx++] = feature;
	gv_framework_features_append(feature);
	gv_framework_configurables_append(feature);
#endif
#ifdef DBUS_SERVER_NATIVE_ENABLED
	feature = gv_feature_new(GV_TYPE_DBUS_SERVER_NATIVE, TRUE);
	features[idx++] = feature;
	gv_framework_features_append(feature);
	gv_framework_configurables_append(feature);
#endif
#ifdef DBUS_SERVER_MPRIS2_ENABLED
	feature = gv_feature_new(GV_TYPE_DBUS_SERVER_MPRIS2, TRUE);
	features[idx++] = feature;
	gv_framework_features_append(feature);
	gv_framework_configurables_append(feature);
#endif
#ifdef INHIBITOR_ENABLED
	feature = gv_feature_new(GV_TYPE_INHIBITOR, FALSE);
	features[idx++] = feature;
	gv_framework_features_append(feature);
	gv_framework_configurables_append(feature);
	gv_framework_errorables_append(feature);
#endif



	/* ----------------------------------------------- *
	 * Initialize global variables                     *
	 * ----------------------------------------------- */

	gv_core_conf = conf;
	gv_core_engine = engine;
	gv_core_station_list = station_list;
	gv_core_player = player;
}

/*
 * Underlying audio backend
 */

#include <gst/gst.h>

#include "additions/gst.h"

void
gv_core_audio_backend_cleanup(void)
{
	if (gst_is_initialized())
		gst_deinit();
}

GOptionGroup *
gv_core_audio_backend_init_get_option_group(void)
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
gv_core_audio_backend_runtime_version_string(void)
{
	return gst_get_runtime_version_string();
}

const gchar *
gv_core_audio_backend_compile_version_string(void)
{
	return gst_get_compile_version_string();
}