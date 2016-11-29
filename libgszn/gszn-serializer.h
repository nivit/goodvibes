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

#ifndef __LIBGSZN_GSZN_SERIALIZER_H__
#define __LIBGSZN_GSZN_SERIALIZER_H__

#include <glib-object.h>

#include "gszn-settings.h"

/* GObject declarations */

#define GSZN_TYPE_SERIALIZER gszn_serializer_get_type()

G_DECLARE_FINAL_TYPE(GsznSerializer, gszn_serializer, GSZN, SERIALIZER, GObject)

/*
 * Flags
 * SERIALIZE_FLAG_ONLY: serialize only properties flagged with SERIALIZE.
 * NON_DEFAULT_ONLY: serialize only properties which value is different from default.
 * WATCH: watch for property changes, emit a signal when a change happened.
 */

typedef enum {
	GSZN_SERIALIZER_FLAG_DEFAULT             = 0,
	GSZN_SERIALIZER_FLAG_SERIALIZE_FLAG_ONLY = (1 << 0),
	GSZN_SERIALIZER_FLAG_NON_DEFAULT_ONLY    = (1 << 1),
	GSZN_SERIALIZER_FLAG_WATCH               = (1 << 2),
} GsznSerializerFlags;

/* Methods */

GsznSerializer *gszn_serializer_new   (GsznSettings *settings, GsznSerializerFlags flags);

gchar *gszn_serializer_print          (GsznSerializer *self);

void   gszn_serializer_add_list       (GsznSerializer *self, GList *objects);
void   gszn_serializer_add_object     (GsznSerializer *self, GObject *object);

void   gszn_serializer_remove_list    (GsznSerializer *self, GList *objects);
void   gszn_serializer_remove_object  (GsznSerializer *self, GObject *object);

#endif /* __LIBGSZN_GSZN_SERIALIZER_H__ */
