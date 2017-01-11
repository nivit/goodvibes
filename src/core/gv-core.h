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

#ifndef __GOODVIBES_CORE_GV_CORE_H__
#define __GOODVIBES_CORE_GV_CORE_H__

#include <glib.h>
#include <gio/gio.h>

#include "core/gv-conf.h"
#include "core/gv-player.h"
#include "core/gv-station.h"
#include "core/gv-station-list.h"

/* Global variables */

extern GvConf        *gv_core_conf;
extern GvPlayer      *gv_core_player;
extern GvStationList *gv_core_station_list;

/* Functions */

void gv_core_init      (GApplication *app);
void gv_core_cleanup   (void);
void gv_core_warm_up   (const gchar *uri_to_play);
void gv_core_cool_down (void);

void gv_core_quit      (void);

/*
 * Underlying audio backend
 */

GOptionGroup *gv_core_audio_backend_init_get_option_group (void);
void          gv_core_audio_backend_cleanup               (void);
const gchar  *gv_core_audio_backend_runtime_version_string(void);
const gchar  *gv_core_audio_backend_compile_version_string(void);

#endif /* __GOODVIBES_CORE_GV_CORE_H__ */
