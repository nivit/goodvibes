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

/*
 * This header contains definitions to be used internally by core files
 */

#ifndef __GOODVIBES_CORE_GV_CORE_INTERNAL_H__
#define __GOODVIBES_CORE_GV_CORE_INTERNAL_H__

#include <gio/gio.h>

/* Global variables */

extern GApplication  *gv_core_application;
extern GSettings     *gv_core_settings;

extern const gchar   *gv_core_user_agent;

#endif /* __GOODVIBES_CORE_GV_CORE_INTERNAL_H__ */
