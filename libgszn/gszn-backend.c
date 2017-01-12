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

#include <glib-object.h>

#include "gszn-backend.h"

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Properties */
	PROP_TITLE,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * GObject definitions
 */

struct _GsznBackendPrivate {
	gchar *title;
};

typedef struct _GsznBackendPrivate GsznBackendPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(GsznBackend, gszn_backend, G_TYPE_OBJECT)

/* Dummy structure to ensure type checking at compile-time.
 * A bit better than using a void* pointer, I guess.
 */
struct _GsznBackendIter {};

/* Methods */

const gchar *
gszn_backend_get_object_uid(GsznBackend *self, GsznBackendIter *iter)
{
	GsznBackendClass *class;

	g_assert(GSZN_IS_BACKEND(self));
	class = GSZN_BACKEND_GET_CLASS(self);

	g_assert(class->get_object_uid != NULL);
	return class->get_object_uid(self, iter);
}

GsznParameter *
gszn_backend_get_properties(GsznBackend *self, GsznBackendIter *iter,
                            guint *n_params)
{
	GsznBackendClass *class;

	g_assert(GSZN_IS_BACKEND(self));
	class = GSZN_BACKEND_GET_CLASS(self);

	g_assert(class->get_properties != NULL);
	return class->get_properties(self, iter, n_params);
}

void
gszn_backend_add_properties(GsznBackend *self, GsznBackendIter *iter,
                            GsznParameter *params, guint n_params)
{
	GsznBackendClass *class;

	g_assert(GSZN_IS_BACKEND(self));
	class = GSZN_BACKEND_GET_CLASS(self);

	g_assert(class->add_properties != NULL);
	class->add_properties(self, iter, params, n_params);
}

GsznBackendIter *
gszn_backend_get_object(GsznBackend *self, const gchar *object_name, const gchar *object_uid)
{
	GsznBackendClass *class;

	g_assert(GSZN_IS_BACKEND(self));
	class = GSZN_BACKEND_GET_CLASS(self);

	g_assert(class->get_object != NULL);
	return class->get_object(self, object_name, object_uid);
}

GsznBackendIter *
gszn_backend_add_object(GsznBackend *self, const gchar *object_name, const gchar *object_uid)
{
	GsznBackendClass *class;

	g_assert(GSZN_IS_BACKEND(self));
	class = GSZN_BACKEND_GET_CLASS(self);

	g_assert(class->add_object != NULL);
	return class->add_object(self, object_name, object_uid);
}

gboolean
gszn_backend_loop(GsznBackend *self, GsznBackendIter *iter, const gchar **object_name)
{
	GsznBackendClass *class;

	g_assert(GSZN_IS_BACKEND(self));
	class = GSZN_BACKEND_GET_CLASS(self);

	g_assert(class->loop != NULL);
	return class->loop(self, iter, object_name);
}

GsznBackendIter *
gszn_backend_get_uninitialized_iter(GsznBackend *self)
{
	GsznBackendClass *class;

	g_assert(GSZN_IS_BACKEND(self));
	class = GSZN_BACKEND_GET_CLASS(self);

	g_assert(class->get_uninitialized_iter != NULL);
	return class->get_uninitialized_iter(self);
}

void
gszn_backend_free_iter(GsznBackend *self, GsznBackendIter *iter)
{
	GsznBackendClass *class;

	g_assert(GSZN_IS_BACKEND(self));
	class = GSZN_BACKEND_GET_CLASS(self);

	g_assert(class->free_iter != NULL);
	class->free_iter(self, iter);
}

gboolean
gszn_backend_parse(GsznBackend *self, const gchar *data, GError **err)
{
	GsznBackendClass *class;

	g_assert(GSZN_IS_BACKEND(self));
	class = GSZN_BACKEND_GET_CLASS(self);

	g_assert(class->parse != NULL);
	return class->parse(self, data, err);
}

gchar *
gszn_backend_print(GsznBackend *self)
{
	GsznBackendClass *class;

	g_assert(GSZN_IS_BACKEND(self));
	class = GSZN_BACKEND_GET_CLASS(self);

	g_assert(class->print != NULL);
	return class->print(self);
}


GsznBackend *
gszn_backend_new(GType backend_type, const gchar *title)
{
	g_assert(g_type_is_a(backend_type, GSZN_TYPE_BACKEND));

	return g_object_new(backend_type, "title", title, NULL);
}

/*
 * Property accessors
 */

const gchar *
gszn_backend_get_title(GsznBackend *self)
{
	GsznBackendPrivate *priv = gszn_backend_get_instance_private(self);

	return priv->title;
}

static void
gszn_backend_set_title(GsznBackend *self, const gchar *title)
{
	GsznBackendPrivate *priv = gszn_backend_get_instance_private(self);

	/* Construct-only property */
	g_assert_null(priv->title);
	priv->title = g_strdup(title);
}

static void
gszn_backend_get_property(GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
	GsznBackend *self = GSZN_BACKEND(object);

	switch (property_id) {
	case PROP_TITLE:
		g_value_set_string(value, gszn_backend_get_title(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
gszn_backend_set_property(GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
	GsznBackend *self = GSZN_BACKEND(object);

	switch (property_id) {
	case PROP_TITLE:
		gszn_backend_set_title(self, g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * GObject methods
 */

static void
gszn_backend_finalize(GObject *object)
{
	GsznBackend *self = GSZN_BACKEND(object);
	GsznBackendPrivate *priv = gszn_backend_get_instance_private(self);

	/* Free resources */
	g_free(priv->title);

	/* Chain up */
	G_OBJECT_CLASS(gszn_backend_parent_class)->finalize(object);
}

static void
gszn_backend_init(GsznBackend *self G_GNUC_UNUSED)
{
}

static void
gszn_backend_class_init(GsznBackendClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	/* Override GObject methods */
	object_class->finalize = gszn_backend_finalize;

	/* Properties */
	object_class->get_property = gszn_backend_get_property;
	object_class->set_property = gszn_backend_set_property;

	properties[PROP_TITLE] =
	        g_param_spec_string("title", "Title", NULL, NULL,
	                            G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE |
	                            G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties(object_class, PROP_N, properties);
}
