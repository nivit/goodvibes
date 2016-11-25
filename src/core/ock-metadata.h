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

#ifndef __OVERCOOKED_CORE_OCK_METADATA_H__
#define __OVERCOOKED_CORE_OCK_METADATA_H__

#include <glib-object.h>

/* GObject declarations */

#define OCK_TYPE_METADATA ock_metadata_get_type()

G_DECLARE_FINAL_TYPE(OckMetadata, ock_metadata, OCK, METADATA, GObject)

/* Methods */

OckMetadata *ock_metadata_new              (void);
gchar       *ock_metadata_make_title_artist(OckMetadata *self, gboolean escape);
gchar       *ock_metadata_make_album_year  (OckMetadata *self, gboolean escape);
gboolean     ock_metadata_is_equal         (OckMetadata *self, OckMetadata *against);

/* Property accessors */

const gchar *ock_metadata_get_title  (OckMetadata *self);
void         ock_metadata_set_title  (OckMetadata *self, const gchar *title);
const gchar *ock_metadata_get_artist (OckMetadata *self);
void         ock_metadata_set_artist (OckMetadata *self, const gchar *artist);
const gchar *ock_metadata_get_album  (OckMetadata *self);
void         ock_metadata_set_album  (OckMetadata *self, const gchar *album);
const gchar *ock_metadata_get_genre  (OckMetadata *self);
void         ock_metadata_set_genre  (OckMetadata *self, const gchar *genre);
const gchar *ock_metadata_get_year   (OckMetadata *self);
void         ock_metadata_set_year   (OckMetadata *self, const gchar *year);
const gchar *ock_metadata_get_comment(OckMetadata *self);
void         ock_metadata_set_comment(OckMetadata *self, const gchar *comment);
guint        ock_metadata_get_bitrate(OckMetadata *self);
void         ock_metadata_set_bitrate(OckMetadata *self, guint bitrate);

#endif /* __OVERCOOKED_CORE_OCK_METADATA_H__ */
