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

#ifndef __OVERCOOKED_CORE_OCK_PLAYLIST_H__
#define __OVERCOOKED_CORE_OCK_PLAYLIST_H__

#include <glib-object.h>

/* GObject declarations */

#define OCK_TYPE_PLAYLIST ock_playlist_get_type()

G_DECLARE_FINAL_TYPE(OckPlaylist, ock_playlist, OCK, PLAYLIST, GObject)

/* Data types */

typedef enum {
	OCK_PLAYLIST_FORMAT_UNKNOWN,
	OCK_PLAYLIST_FORMAT_M3U,
	OCK_PLAYLIST_FORMAT_PLS,
	OCK_PLAYLIST_FORMAT_ASX,
	OCK_PLAYLIST_FORMAT_XSPF
} OckPlaylistFormat;

/* Class methods */

OckPlaylistFormat ock_playlist_get_format(const gchar *uri);

/* Methods */

OckPlaylist *ock_playlist_new     (const gchar *uri);
void         ock_playlist_download(OckPlaylist *playlist);

/* Property accessors */

const gchar *ock_playlist_get_uri        (OckPlaylist *self);
void         ock_playlist_set_uri        (OckPlaylist *self, const gchar *uri);
GSList      *ock_playlist_get_stream_list(OckPlaylist *playlist);

#endif /* __OVERCOOKED_CORE_OCK_PLAYLIST_H__ */
