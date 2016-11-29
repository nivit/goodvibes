/*
 * Libgszn
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

#ifndef __LIBGSZN_GSZN_DEBUG_H__
#define __LIBGSZN_GSZN_DEBUG_H__

#include <glib.h>

#ifdef GSZN_ENABLE_DEBUG
#define GSZN_DEBUG_ENABLED 1
#else
#define GSZN_DEBUG_ENABLED 0
#endif

#define GSZN_DEBUG(fmt, ...) \
	do { if (GSZN_DEBUG_ENABLED) g_debug(fmt, ##__VA_ARGS__); } while (0)

#endif /* __LIBGSZN_GSZN_DEBUG_H__ */
