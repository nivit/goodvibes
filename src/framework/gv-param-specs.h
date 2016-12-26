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

#ifndef __GOODVIBES_FRAMEWORK_GV_PARAM_SPECS_H__
#define __GOODVIBES_FRAMEWORK_GV_PARAM_SPECS_H__

#include <glib-object.h>

/* Default flags for objects:
 * - STATIC_STRINGS because we only use static strings.
 * - EXPLICIT_NOTIFY because we want to notify only when a property
 *   is changed. It's a design choice, every object should stick to it,
 *   otherwise expect surprises.
 */

#define GV_PARAM_DEFAULT_FLAGS G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY

#endif /* __GOODVIBES_FRAMEWORK_GV_PARAM_SPECS_H__ */
