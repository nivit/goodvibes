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

#ifndef __OVERCOOKED_CORE_OCK_CORE_H__
#define __OVERCOOKED_CORE_OCK_CORE_H__

#include "core/ock-conf.h"
#include "core/ock-player.h"
#include "core/ock-station.h"
#include "core/ock-station-list.h"

/* Global variables, valid after create(), invalid after destroy() */
extern OckConf        *ock_core_conf;
extern OckPlayer      *ock_core_player;
extern OckStationList *ock_core_station_list;

void ock_core_init     (void);
void ock_core_cleanup  (void);
void ock_core_warm_up  (const gchar *uri_to_play);
void ock_core_cool_down(void);

/*
 * Underlying audio backend
 */

GOptionGroup *ock_core_audio_backend_init_get_option_group (void);
void          ock_core_audio_backend_cleanup               (void);
const gchar  *ock_core_audio_backend_runtime_version_string(void);
const gchar  *ock_core_audio_backend_compile_version_string(void);

#endif /* __OVERCOOKED_CORE_OCK_CORE_H__ */
