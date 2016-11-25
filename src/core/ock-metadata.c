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

#include <glib.h>
#include <glib-object.h>

#include "additions/glib-object.h"

#include "framework/log.h"
#include "framework/ock-param-specs.h"

#include "core/ock-metadata.h"

// TODO Does this really need to be an object ? Maybe a simple boxed structure would do.

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

struct _OckMetadataPrivate {
	gchar *title;
	gchar *artist;
	gchar *album;
	gchar *genre;
	gchar *year;
	gchar *comment;
	guint  bitrate;
};

typedef struct _OckMetadataPrivate OckMetadataPrivate;

struct _OckMetadata {
	/* Parent instance structure */
	GObject             parent_instance;
	/* Private data */
	OckMetadataPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(OckMetadata, ock_metadata, G_TYPE_OBJECT)

/*
 * Property accessors
 */

const gchar *
ock_metadata_get_title(OckMetadata *self)
{
	return self->priv->title;
}

void
ock_metadata_set_title(OckMetadata *self, const gchar *title)
{
	OckMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->title, title))
		return;

	g_free(priv->title);
	priv->title = g_strdup(title);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_TITLE]);
}

const gchar *
ock_metadata_get_artist(OckMetadata *self)
{
	return self->priv->artist;
}

void
ock_metadata_set_artist(OckMetadata *self, const gchar *artist)
{
	OckMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->artist, artist))
		return;

	g_free(priv->artist);
	priv->artist = g_strdup(artist);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_ARTIST]);
}
const gchar *
ock_metadata_get_album(OckMetadata *self)
{
	return self->priv->album;
}

void
ock_metadata_set_album(OckMetadata *self, const gchar *album)
{
	OckMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->album, album))
		return;

	g_free(priv->album);
	priv->album = g_strdup(album);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_ALBUM]);
}

const gchar *
ock_metadata_get_genre(OckMetadata *self)
{
	return self->priv->genre;
}

void
ock_metadata_set_genre(OckMetadata *self, const gchar *genre)
{
	OckMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->genre, genre))
		return;

	g_free(priv->genre);
	priv->genre = g_strdup(genre);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_GENRE]);
}

const gchar *
ock_metadata_get_year(OckMetadata *self)
{
	return self->priv->year;
}

void
ock_metadata_set_year(OckMetadata *self, const gchar *year)
{
	OckMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->year, year))
		return;

	g_free(priv->year);
	priv->year = g_strdup(year);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_YEAR]);
}

const gchar *
ock_metadata_get_comment(OckMetadata *self)
{
	return self->priv->comment;
}

void
ock_metadata_set_comment(OckMetadata *self, const gchar *comment)
{
	OckMetadataPrivate *priv = self->priv;

	if (!g_strcmp0(priv->comment, comment))
		return;

	g_free(priv->comment);
	priv->comment = g_strdup(comment);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_COMMENT]);
}

guint
ock_metadata_get_bitrate(OckMetadata *self)
{
	return self->priv->bitrate;
}

void
ock_metadata_set_bitrate(OckMetadata *self, guint bitrate)
{
	OckMetadataPrivate *priv = self->priv;

	if (priv->bitrate == bitrate)
		return;

	priv->bitrate = bitrate;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_BITRATE]);
}

static void
ock_metadata_get_property(GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
	OckMetadata *self = OCK_METADATA(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_TITLE:
		g_value_set_string(value, ock_metadata_get_title(self));
		break;
	case PROP_ARTIST:
		g_value_set_string(value, ock_metadata_get_artist(self));
		break;
	case PROP_ALBUM:
		g_value_set_string(value, ock_metadata_get_album(self));
		break;
	case PROP_GENRE:
		g_value_set_string(value, ock_metadata_get_genre(self));
		break;
	case PROP_YEAR:
		g_value_set_string(value, ock_metadata_get_year(self));
		break;
	case PROP_COMMENT:
		g_value_set_string(value, ock_metadata_get_comment(self));
		break;
	case PROP_BITRATE:
		g_value_set_uint(value, ock_metadata_get_bitrate(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
ock_metadata_set_property(GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
	OckMetadata *self = OCK_METADATA(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_TITLE:
		ock_metadata_set_title(self, g_value_get_string(value));
		break;
	case PROP_ARTIST:
		ock_metadata_set_artist(self, g_value_get_string(value));
		break;
	case PROP_ALBUM:
		ock_metadata_set_album(self, g_value_get_string(value));
		break;
	case PROP_GENRE:
		ock_metadata_set_genre(self, g_value_get_string(value));
		break;
	case PROP_YEAR:
		ock_metadata_set_year(self, g_value_get_string(value));
		break;
	case PROP_COMMENT:
		ock_metadata_set_comment(self, g_value_get_string(value));
		break;
	case PROP_BITRATE:
		ock_metadata_set_bitrate(self, g_value_get_uint(value));
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
ock_metadata_is_equal(OckMetadata *self, OckMetadata *against)
{
	OckMetadataPrivate *priv1 = self->priv;
	OckMetadataPrivate *priv2 = against->priv;

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
ock_metadata_make_title_artist(OckMetadata *self, gboolean escape)
{
	OckMetadataPrivate *priv = self->priv;
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
ock_metadata_make_album_year(OckMetadata *self, gboolean escape)
{
	OckMetadataPrivate *priv = self->priv;
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

OckMetadata *
ock_metadata_new(void)
{
	return g_object_new(OCK_TYPE_METADATA, NULL);
}

/*
 * GObject methods
 */

static void
ock_metadata_finalize(GObject *object)
{
	OckMetadataPrivate *priv = OCK_METADATA(object)->priv;

	TRACE("%p", object);

	/* Free any allocated resources */
	g_free(priv->title);
	g_free(priv->artist);
	g_free(priv->album);
	g_free(priv->genre);
	g_free(priv->year);
	g_free(priv->comment);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_metadata, object);
}

static void
ock_metadata_init(OckMetadata *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_metadata_get_instance_private(self);
}

static void
ock_metadata_class_init(OckMetadataClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_metadata_finalize;

	/* Properties */
	object_class->get_property = ock_metadata_get_property;
	object_class->set_property = ock_metadata_set_property;

	properties[PROP_TITLE] =
	        g_param_spec_string("title", "Title", NULL, NULL,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_ARTIST] =
	        g_param_spec_string("artist", "Artist", NULL, NULL,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_ALBUM] =
	        g_param_spec_string("album", "Album", NULL, NULL,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_GENRE] =
	        g_param_spec_string("genre", "Genre", NULL, NULL,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_YEAR] =
	        g_param_spec_string("year", "Year", NULL, NULL,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_COMMENT] =
	        g_param_spec_string("comment", "Comment", NULL, NULL,
	                            OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	properties[PROP_BITRATE] =
	        g_param_spec_uint("bitrate", "Bitrate", NULL, 0, G_MAXUINT, 0,
	                          OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE);

	g_object_class_install_properties(object_class, PROP_N, properties);
}
