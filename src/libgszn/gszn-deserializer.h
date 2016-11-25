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

#ifndef __OVERCOOKED_LIBGSZN_GSZN_DESERIALIZER_H__
#define __OVERCOOKED_LIBGSZN_GSZN_DESERIALIZER_H__

#include <glib-object.h>

#include "libgszn/gszn-settings.h"

/* GObject declarations */

#define GSZN_TYPE_DESERIALIZER gszn_deserializer_get_type()

G_DECLARE_FINAL_TYPE(GsznDeserializer, gszn_deserializer, GSZN, DESERIALIZER, GObject)

/* Methods */

GsznDeserializer *gszn_deserializer_new  (GsznSettings *settings);

gboolean gszn_deserializer_parse         (GsznDeserializer *self, const gchar *data,
                                          GError **err);
GList   *gszn_deserializer_create_all    (GsznDeserializer *self);
GObject *gszn_deserializer_create_object (GsznDeserializer *self, const gchar *object_name,
                                          const gchar *object_uid);
void     gszn_deserializer_restore_all   (GsznDeserializer *self, GList *objects);
void     gszn_deserializer_restore_object(GsznDeserializer *self, GObject *object);

#endif /* __OVERCOOKED_LIBGSZN_GSZN_DESERIALIZER_H__ */
