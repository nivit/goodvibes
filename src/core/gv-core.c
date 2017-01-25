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

#include "framework/gv-framework.h"

#include "core/gv-engine.h"
#include "core/gv-player.h"
#include "core/gv-station-list.h"

GApplication  *gv_core_application;
GSettings     *gv_core_settings;

GvStationList *gv_core_station_list;
GvPlayer      *gv_core_player;

static GvEngine *gv_core_engine;

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
	return gst_init_get_option_group();
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

/*
 * Core public functions
 */

void
gv_core_quit(void)
{
	g_application_quit(gv_core_application);
}

void
gv_core_cleanup(void)
{
	/* Destroy core objects */

	g_object_unref(gv_core_player);
	g_object_unref(gv_core_station_list);
	g_object_unref(gv_core_engine);

	/* Destroy settings */

	g_object_unref(gv_core_settings);

	/* Clean application pointer */

	gv_core_application = NULL;
}

void
gv_core_init(GApplication *application)
{
	/* Keep a pointer toward application */

	gv_core_application = application;

	/* Create settings */

	gv_core_settings = g_settings_new(PACKAGE_APPLICATION_ID ".Core");
	gv_framework_register(gv_core_settings);

	/* Create core objects */

	gv_core_engine = gv_engine_new();
	gv_framework_register(gv_core_engine);

	gv_core_station_list = gv_station_list_new();
	gv_framework_register(gv_core_station_list);

	gv_station_list_load(gv_core_station_list);

	gv_core_player = gv_player_new(gv_core_engine, gv_core_station_list);
	gv_framework_register(gv_core_player);
}
