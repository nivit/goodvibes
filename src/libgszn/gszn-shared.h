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

#ifndef __OVERCOOKED_LIBGSZN_GSZN_SHARED_H__
#define __OVERCOOKED_LIBGSZN_GSZN_SHARED_H__

#include <glib-object.h>

/* Transform functions */

gboolean gszn_transform_string_to_value(const gchar *string, GValue *value, GError **err);
gboolean gszn_transform_value_to_string(const GValue *value, gchar **string, GError **err);

/* GObject helpers */

gchar *gszn_get_object_uid(GObject *object);
void   gszn_set_object_uid(GObject *object, const gchar *uid);

#endif /* __OVERCOOKED_LIBGSZN_GSZN_SHARED_H__ */
