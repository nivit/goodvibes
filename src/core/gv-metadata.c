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

#include <glib.h>
#include <glib-object.h>

#include "additions/glib-object.h"

#include "framework/gv-framework.h"

#include "core/gv-metadata.h"

// WISHED I think this doesn't need to be a GObject. A simple boxed type would do.

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Properties */
	PROP_TITLE,
	PROP_ARTIST,
	PROP_ALBUM,
	PROP_GENRE,
	PROP_YEAR,
	PROP_COMMENT,
	PROP_BITRATE,
	// WISHED Label ?
	// WISHED Cover ?
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * GObject definitions
 */

struct _GvMetadataPrivate {
	gchar *title;
	gchar *artist;
	gchar *album;
	gchar *genre;
	gchar *year;
	gchar *comment;
	guint  bitrate;
};

typedef struct _GvMetadataPrivate GvMetadataPrivate;

struct _GvMetadata {
	/* Parent instance structure */
	GObject             parent_instance;
	/* Private data */
	GvMetadataPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(GvMetadata, gv_metadata, G_TYPE_OBJECT)

/*
 * Property accessors
 */

const gchar *
gv_metadata_get_title(GvMetadata *self)
{
	return self->priv->title;
}

void
gv_metadata_set_title(GvMetadata *self, const gchar *title)
{
	GvMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->title, title))
		return;

	g_free(priv->title);
	priv->title = g_strdup(title);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_TITLE]);
}

const gchar *
gv_metadata_get_artist(GvMetadata *self)
{
	return self->priv->artist;
}

void
gv_metadata_set_artist(GvMetadata *self, const gchar *artist)
{
	GvMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->artist, artist))
		return;

	g_free(priv->artist);
	priv->artist = g_strdup(artist);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_ARTIST]);
}
const gchar *
gv_metadata_get_album(GvMetadata *self)
{
	return self->priv->album;
}

void
gv_metadata_set_album(GvMetadata *self, const gchar *album)
{
	GvMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->album, album))
		return;

	g_free(priv->album);
	priv->album = g_strdup(album);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_ALBUM]);
}

const gchar *
gv_metadata_get_genre(GvMetadata *self)
{
	return self->priv->genre;
}

void
gv_metadata_set_genre(GvMetadata *self, const gchar *genre)
{
	GvMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->genre, genre))
		return;

	g_free(priv->genre);
	priv->genre = g_strdup(genre);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_GENRE]);
}

const gchar *
gv_metadata_get_year(GvMetadata *self)
{
	return self->priv->year;
}

void
gv_metadata_set_year(GvMetadata *self, const gchar *year)
{
	GvMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->year, year))
		return;

	g_free(priv->year);
	priv->year = g_strdup(year);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_YEAR]);
}

const gchar *
gv_metadata_get_comment(GvMetadata *self)
{
	return self->priv->comment;
}

void
gv_metadata_set_comment(GvMetadata *self, const gchar *comment)
{
	GvMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->comment, comment))
		return;

	g_free(priv->comment);
	priv->comment = g_strdup(comment);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_COMMENT]);
}

guint
gv_metadata_get_bitrate(GvMetadata *self)
{
	return self->priv->bitrate;
}

void
gv_metadata_set_bitrate(GvMetadata *self, guint bitrate)
{
	GvMetadataPrivate *priv = self->priv;

	if (priv->bitrate == bitrate)
		return;

	priv->bitrate = bitrate;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_BITRATE]);
}

static void
gv_metadata_get_property(GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
	GvMetadata *self = GV_METADATA(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_TITLE:
		g_value_set_string(value, gv_metadata_get_title(self));
		break;
	case PROP_ARTIST:
		g_value_set_string(value, gv_metadata_get_artist(self));
		break;
	case PROP_ALBUM:
		g_value_set_string(value, gv_metadata_get_album(self));
		break;
	case PROP_GENRE:
		g_value_set_string(value, gv_metadata_get_genre(self));
		break;
	case PROP_YEAR:
		g_value_set_string(value, gv_metadata_get_year(self));
		break;
	case PROP_COMMENT:
		g_value_set_string(value, gv_metadata_get_comment(self));
		break;
	case PROP_BITRATE:
		g_value_set_uint(value, gv_metadata_get_bitrate(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
gv_metadata_set_property(GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
	GvMetadata *self = GV_METADATA(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_TITLE:
		gv_metadata_set_title(self, g_value_get_string(value));
		break;
	case PROP_ARTIST:
		gv_metadata_set_artist(self, g_value_get_string(value));
		break;
	case PROP_ALBUM:
		gv_metadata_set_album(self, g_value_get_string(value));
		break;
	case PROP_GENRE:
		gv_metadata_set_genre(self, g_value_get_string(value));
		break;
	case PROP_YEAR:
		gv_metadata_set_year(self, g_value_get_string(value));
		break;
	case PROP_COMMENT:
		gv_metadata_set_comment(self, g_value_get_string(value));
		break;
	case PROP_BITRATE:
		gv_metadata_set_bitrate(self, g_value_get_uint(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Public methods
 */

gboolean
gv_metadata_is_equal(GvMetadata *self, GvMetadata *against)
{
	GvMetadataPrivate *priv1 = self->priv;
	GvMetadataPrivate *priv2 = against->priv;

	if (g_strcmp0(priv1->title, priv2->title))
		return FALSE;

	if (g_strcmp0(priv1->artist, priv2->artist))
		return FALSE;

	if (g_strcmp0(priv1->album, priv2->album))
		return FALSE;

	if (g_strcmp0(priv1->genre, priv2->genre))
		return FALSE;

	if (g_strcmp0(priv1->year, priv2->year))
		return FALSE;

	if (g_strcmp0(priv1->comment, priv2->comment))
		return FALSE;

	if (priv1->bitrate != priv2->bitrate)
		return FALSE;

	return TRUE;
}

gchar *
gv_metadata_make_title_artist(GvMetadata *self, gboolean escape)
{
	GvMetadataPrivate *priv = self->priv;
	gchar *str, *str2;

	if (priv->artist && priv->title)
		str = g_strdup_printf("%s - %s", priv->title, priv->artist);
	else if (priv->title)
		str = g_strdup_printf("%s", priv->title);
	else if (priv->artist)
		str = g_strdup_printf("%s", priv->artist);
	else
		str = NULL;

	if (str && escape == TRUE) {
		str2 = g_markup_escape_text(str, -1);
		g_free(str);
		str = str2;
	}

	return str;
}

gchar *
gv_metadata_make_album_year(GvMetadata *self, gboolean escape)
{
	GvMetadataPrivate *priv = self->priv;
	gchar *str, *str2;

	if (priv->album && priv->year)
		str = g_strdup_printf("%s (%s)", priv->album, priv->year);
	else if (priv->album)
		str = g_strdup_printf("%s", priv->album);
	else if (priv->year)
		str = g_strdup_printf("(%s)", priv->year);
	else
		str = NULL;

	if (str && escape == TRUE) {
		str2 = g_markup_escape_text(str, -1);
		g_free(str);
		str = str2;
	}

	return str;
}

GvMetadata *
gv_metadata_new(void)
{
	return g_object_new(GV_TYPE_METADATA, NULL);
}

/*
 * GObject methods
 */

static void
gv_metadata_finalize(GObject *object)
{
	GvMetadataPrivate *priv = GV_METADATA(object)->priv;

	TRACE("%p", object);

	/* Free any allocated resources */
	g_free(priv->title);
	g_free(priv->artist);
	g_free(priv->album);
	g_free(priv->genre);
	g_free(priv->year);
	g_free(priv->comment);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(gv_metadata, object);
}

static void
gv_metadata_init(GvMetadata *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = gv_metadata_get_instance_private(self);
}

static void
gv_metadata_class_init(GvMetadataClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = gv_metadata_finalize;

	/* Properties */
	object_class->get_property = gv_metadata_get_property;
	object_class->set_property = gv_metadata_set_property;

	properties[PROP_TITLE] =
	        g_param_spec_string("title", "Title", NULL, NULL,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_ARTIST] =
	        g_param_spec_string("artist", "Artist", NULL, NULL,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_ALBUM] =
	        g_param_spec_string("album", "Album", NULL, NULL,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_GENRE] =
	        g_param_spec_string("genre", "Genre", NULL, NULL,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_YEAR] =
	        g_param_spec_string("year", "Year", NULL, NULL,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_COMMENT] =
	        g_param_spec_string("comment", "Comment", NULL, NULL,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_BITRATE] =
	        g_param_spec_uint("bitrate", "Bitrate", NULL, 0, G_MAXUINT, 0,
	                          GV_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	g_object_class_install_properties(object_class, PROP_N, properties);
}
