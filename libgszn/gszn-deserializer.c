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

#include <glib.h>
#include <glib-object.h>

#include "gszn-backend.h"
#include "gszn-deserializer.h"
#include "gszn-shared.h"

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Construct-only properties */
	PROP_SETTINGS,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * GObject definitions
 */

struct _GsznDeserializerPrivate {
	/* Construct-only properties */
	GsznSettings *settings;
	/* Internal */
	GsznBackend  *backend;
};

typedef struct _GsznDeserializerPrivate GsznDeserializerPrivate;

struct _GsznDeserializer {
	/* Parent instance structure */
	GObject parent_instance;
	/* Private data */
	GsznDeserializerPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(GsznDeserializer, gszn_deserializer, G_TYPE_OBJECT)

/*
 * Helpers
 */

static GParameter *
make_object_parameters(GsznBackend *backend, GsznBackendIter *iter, GType object_type,
                       guint *n_params, GsznTweakString tweak_property_name)
{
	GObjectClass *object_class;
	GsznParameter *ser_params;
	guint n_ser_params;
	GParameter *params;
	guint i, j;

	g_return_val_if_fail(n_params != NULL, NULL);

	/* Get object class */
	object_class = g_type_class_peek(object_type);
	if (object_class == NULL) {
		g_debug("Object type '%s' not registered", g_type_name(object_type));
		return NULL;
	}

	/* Get serialized values */
	ser_params = gszn_backend_get_properties(backend, iter, &n_ser_params);

	/* Allocate parameters */
	params = g_new0(GParameter, n_ser_params);

	/* Iterate and transform every serialized param into a GValue param */
	for (i = 0, j = 0; i < n_ser_params; i++) {
		GsznParameter *ser_param = &ser_params[i];
		GParameter *param = &params[j];
		GParamSpec *pspec;
		gchar *property_name;
		GError *err = NULL;

		/* Deserialize property name */
		if (tweak_property_name)
			property_name = tweak_property_name(ser_param->name);
		else
			property_name = g_strdup(ser_param->name);

		/* Find corresponding property on object */
		pspec = g_object_class_find_property(object_class, property_name);
		if (!pspec) {
			g_debug("'%s:%s' not found", g_type_name(object_type), property_name);
			goto next_iteration;
		}

		/* Deserialize the value */
		g_value_init(&param->value, pspec->value_type);
		gszn_transform_string_to_value(ser_param->string, &param->value, &err);
		if (err) {
			g_debug("Failed to transform '%s:%s' to value: %s",
			        g_type_name(object_type), property_name, err->message);
			g_clear_error(&err);
			g_value_unset(&param->value);
			goto next_iteration;
		}

		/* Set parameter name - GParameter needs a static string, so we take
		 * care to set the one from the pspec.
		 */
		param->name = pspec->name;

		/* One more param */
		j++;

	next_iteration:
		/* Free temporary property name */
		g_free(property_name);

		/* Consume the serialized parameter */
		g_free(ser_param->name);
		g_free(ser_param->string);
	}

	/* Cleanup */
	g_free(ser_params);

	/* Time to return */
	*n_params = j;
	return params;
}

static void
restore_object(GsznBackend *backend, GsznBackendIter *iter, GObject *object,
               GsznTweakString tweak_property_name)
{
	GParameter *params;
	guint n_params;
	guint i;

	/* Get serialized parameters */
	params = make_object_parameters(backend, iter, G_OBJECT_TYPE(object), &n_params,
	                                tweak_property_name);

	/* Configure the object (consume parameters) */
	for (i = 0; i < n_params; i++) {
		GParameter *param = &params[i];

		g_object_set_property(object, param->name, &param->value);
		g_value_unset(&param->value);
	}

	/* Cleanup */
	g_free(params);
}

static GObject *
create_object(GsznBackend *backend, GsznBackendIter *iter, GType object_type,
              GsznTweakString tweak_property_name)
{
	GObject *object;
	const gchar *object_uid;
	GParameter *params;
	guint n_params;
	guint i;

	/* Get serialized parameters */
	params = make_object_parameters(backend, iter, object_type, &n_params,
	                                tweak_property_name);
	object_uid = gszn_backend_get_object_uid(backend, iter);

	/* Create the object */
	object = g_object_newv(object_type, n_params, params);

	/* Set the object uid */
	if (object_uid)
		gszn_set_object_uid(object, object_uid);

	/* Cleanup */
	for (i = 0 ; i < n_params; i++) {
		GParameter *param = &params[i];

		g_value_unset(&param->value);
	}
	g_free(params);

	return object;
}

/*
 * Methods
 */

void
gszn_deserializer_restore_object(GsznDeserializer *self, GObject *object)
{
	GsznDeserializerPrivate *priv = self->priv;
	GsznBackend *backend = priv->backend;
	GsznBackendIter *iter;
	gchar *object_name;
	gchar *object_uid;

	/* Serialize object name */
	if (priv->settings->ser_object_name)
		object_name = priv->settings->ser_object_name(G_OBJECT_TYPE_NAME(object));
	else
		object_name = g_strdup(G_OBJECT_TYPE_NAME(object));

	/* Get objecy uid */
	object_uid = gszn_get_object_uid(object);

	/* Get object iter */
	iter = gszn_backend_get_object(backend, object_name, object_uid);
	if (iter == NULL)
		goto cleanup;

	/* Restore the object from serialization */
	restore_object(backend, iter, object, priv->settings->deser_property_name);

	/* Cleanup */
	gszn_backend_free_iter(backend, iter);
cleanup:
	g_free(object_uid);
	g_free(object_name);
}

void
gszn_deserializer_restore_all(GsznDeserializer *self, GList *objects)
{
	GList *item;

	for (item = objects; item; item = item->next) {
		GObject *object;

		g_assert(G_IS_OBJECT(item->data));
		object = G_OBJECT(item->data);

		gszn_deserializer_restore_object(self, object);
	}
}

GObject *
gszn_deserializer_create_object(GsznDeserializer *self, const gchar *object_name,
                                const gchar *object_uid)
{
	(void) self;
	(void) object_name;
	(void) object_uid;

	g_warning("Not implemented !");

	return NULL;
}

/*
 * Please notice that the types you intend to deserialize must have been
 * registered beforehand, probably using g_type_class_ref().
 */

GList *
gszn_deserializer_create_all(GsznDeserializer *self)
{
	GsznDeserializerPrivate *priv = self->priv;
	GsznBackend *backend = priv->backend;
	GsznBackendIter *iter;
	const gchar *object_name;
	GList *list = NULL;

	/* Create an iterator */
	iter = gszn_backend_get_uninitialized_iter(backend);

	/* Loop and deserialize every object */
	while (gszn_backend_loop(backend, iter, &object_name)) {
		gchar *object_type_name;
		GType object_type;
		GObject *object;

		/* Deserialize object name */
		if (priv->settings->deser_object_name)
			object_type_name = priv->settings->deser_object_name(object_name);
		else
			object_type_name = g_strdup(object_name);

		/* Find type according to name */
		object_type = g_type_from_name(object_type_name);
		if (object_type == G_TYPE_INVALID) {
			g_debug("Invalid object type name '%s'", object_type_name);
			goto next_iteration;
		}

		/* Deserialize the object */
		object = create_object(backend, iter, object_type,
		                       priv->settings->deser_property_name);

		/* Add the object to the list */
		list = g_list_prepend(list, object);

	next_iteration:
		g_free(object_type_name);
	}

	/* Cleanup */
	gszn_backend_free_iter(backend, iter);

	/* Invert and return the object list */
	list = g_list_reverse(list);

	return list;
}

gboolean
gszn_deserializer_parse(GsznDeserializer *self, const gchar *data, GError **err)
{
	GsznDeserializerPrivate *priv = self->priv;

	g_return_val_if_fail(err == NULL || *err == NULL, FALSE);

	return gszn_backend_parse(priv->backend, data, err);
}

GsznDeserializer *
gszn_deserializer_new(GsznSettings *settings)
{
	return g_object_new(GSZN_TYPE_DESERIALIZER,
	                    "settings", settings,
	                    NULL);
}

/*
 * Property accessors
 */

static void
gszn_deserializer_get_property(GObject    *object,
                               guint       property_id,
                               GValue     *value G_GNUC_UNUSED,
                               GParamSpec *pspec)
{
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
gszn_deserializer_set_property(GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
	GsznDeserializer *self = GSZN_DESERIALIZER(object);
	GsznDeserializerPrivate *priv = self->priv;

	switch (property_id) {
	case PROP_SETTINGS:
		priv->settings = g_value_dup_boxed(value);
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
gszn_deserializer_finalize(GObject *object)
{
	GsznDeserializer *self = GSZN_DESERIALIZER(object);
	GsznDeserializerPrivate *priv = self->priv;

	/* Free resources */
	gszn_settings_free(priv->settings);
	g_object_unref(priv->backend);

	/* Chain up */
	G_OBJECT_CLASS(gszn_deserializer_parent_class)->finalize(object);
}

static void
gszn_deserializer_constructed(GObject *object)
{
	GsznDeserializer *self = GSZN_DESERIALIZER(object);
	GsznDeserializerPrivate *priv = self->priv;

	/* Settings are mandatory */
	g_assert(priv->settings != NULL);

	/* Create the deserialization backend */
	priv->backend = g_object_new(priv->settings->backend_type, NULL);

	/* Chain up */
	if (G_OBJECT_CLASS(gszn_deserializer_parent_class)->constructed)
		G_OBJECT_CLASS(gszn_deserializer_parent_class)->constructed(object);
}

static void
gszn_deserializer_init(GsznDeserializer *self)
{
	/* Initialize private pointer */
	self->priv = gszn_deserializer_get_instance_private(self);
}

static void
gszn_deserializer_class_init(GsznDeserializerClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	/* Override GObject methods */
	object_class->finalize = gszn_deserializer_finalize;
	object_class->constructed = gszn_deserializer_constructed;

	/* Properties */
	object_class->get_property = gszn_deserializer_get_property;
	object_class->set_property = gszn_deserializer_set_property;

	properties[PROP_SETTINGS] =
	        g_param_spec_boxed("settings", "Settings", NULL,
	                           GSZN_TYPE_SETTINGS,
	                           G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE |
	                           G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties(object_class, PROP_N, properties);
}
