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

#ifndef __GOODVIBES_CORE_GV_METADATA_H__
#define __GOODVIBES_CORE_GV_METADATA_H__

#include <glib-object.h>

/* GObject declarations */

#define GV_TYPE_METADATA gv_metadata_get_type()

G_DECLARE_FINAL_TYPE(GvMetadata, gv_metadata, GV, METADATA, GObject)

/* Methods */

GvMetadata *gv_metadata_new              (void);
gchar       *gv_metadata_make_title_artist(GvMetadata *self, gboolean escape);
gchar       *gv_metadata_make_album_year  (GvMetadata *self, gboolean escape);
gboolean     gv_metadata_is_equal         (GvMetadata *self, GvMetadata *against);

/* Property accessors */

const gchar *gv_metadata_get_title  (GvMetadata *self);
void         gv_metadata_set_title  (GvMetadata *self, const gchar *title);
const gchar *gv_metadata_get_artist (GvMetadata *self);
void         gv_metadata_set_artist (GvMetadata *self, const gchar *artist);
const gchar *gv_metadata_get_album  (GvMetadata *self);
void         gv_metadata_set_album  (GvMetadata *self, const gchar *album);
const gchar *gv_metadata_get_genre  (GvMetadata *self);
void         gv_metadata_set_genre  (GvMetadata *self, const gchar *genre);
const gchar *gv_metadata_get_year   (GvMetadata *self);
void         gv_metadata_set_year   (GvMetadata *self, const gchar *year);
const gchar *gv_metadata_get_comment(GvMetadata *self);
void         gv_metadata_set_comment(GvMetadata *self, const gchar *comment);
guint        gv_metadata_get_bitrate(GvMetadata *self);
void         gv_metadata_set_bitrate(GvMetadata *self, guint bitrate);

#endif /* __GOODVIBES_CORE_GV_METADATA_H__ */
