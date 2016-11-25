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

#ifndef __OVERCOOKED_LIBGSZN_GSZN_PARAM_SPECS_H__
#define __OVERCOOKED_LIBGSZN_GSZN_PARAM_SPECS_H__

#include <glib-object.h>

/* Use this flag on GObject properties to signal they're to be serialized */

#define GSZN_PARAM_SERIALIZE (1 << (G_PARAM_USER_SHIFT + 1))

#endif /* __OVERCOOKED_LIBGSZN_GSZN_PARAM_SPECS_H__ */
