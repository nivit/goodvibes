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
#include "gszn-enum-types.h"
#include "gszn-param-specs.h"
#include "gszn-serializer.h"
#include "gszn-settings.h"
#include "gszn-shared.h"

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Construct-only properties */
	PROP_SETTINGS,
	PROP_FLAGS,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * Signals
 */

enum {
	SIGNAL_OBJECT_CHANGED,
	/* Number of signals */
	SIGNAL_N
};

static guint signals[SIGNAL_N];

/*
 * GObject definitions
 */

struct _GsznSerializerPrivate {
	/* Construct-only properties */
	GsznSettings       *settings;
	GsznSerializerFlags flags;
	/* Flags */
	gboolean  serialize_flag_only;
	gboolean  non_default_only;
	gboolean  watch;
	/* Object list */
	GList    *objects;
};

typedef struct _GsznSerializerPrivate GsznSerializerPrivate;

struct _GsznSerializer {
	/* Parent instance structure */
	GObject parent_instance;
	/* Private data */
	GsznSerializerPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(GsznSerializer, gszn_serializer, G_TYPE_OBJECT)

/*
 * Serialization helpers
 */

static gboolean
is_property_type_serializable(const GParamSpec *pspec)
{
	GType pspec_type;

	pspec_type = G_PARAM_SPEC_VALUE_TYPE(pspec);

	/* If the type can be transformed to string, that's ok */
	if (g_value_type_transformable(pspec_type, G_TYPE_STRING))
		return TRUE;

	/* Otherwise, we know that we can serialize some simple types.
	 * We assume here that GObject will never reorder its types.
	 */
	if (pspec_type >= G_TYPE_CHAR && pspec_type <= G_TYPE_STRING)
		return TRUE;

	/* The type might also be derived from ENUM or FLAGS */
	if (g_type_is_a(pspec_type, G_TYPE_ENUM) ||
	    g_type_is_a(pspec_type, G_TYPE_FLAGS))
		return TRUE;

	return FALSE;
}

#define is_property_readable(pspec)     \
	(pspec->flags & G_PARAM_READABLE)

#define is_property_deprecated(pspec)   \
	(pspec->flags & G_PARAM_DEPRECATED)

#define is_property_serialization_wanted(pspec, serialize_flag_only)    \
	!(serialize_flag_only && !(pspec->flags & GSZN_PARAM_SERIALIZE))

static gboolean
value_is_default(const GValue *value, GParamSpec *pspec)
{
	const GValue *default_value;

	default_value = g_param_spec_get_default_value(pspec);
	if (g_param_values_cmp(pspec, value, default_value) == 0)
		return TRUE;

	return FALSE;
}

static GsznParameter *
make_serialized_parameters(GObject *object, guint *n_params, gboolean serialize_flag_only,
                           gboolean non_default_only, GsznTweakString tweak_property_name)
{
	GObjectClass *object_class;
	GParamSpec **object_pspecs;
	guint n_pspecs;
	GsznParameter *ser_params;
	guint i, j;

	g_return_val_if_fail(n_params != NULL, NULL);

	/* Get properties list */
	object_class = G_OBJECT_GET_CLASS(object);
	object_pspecs = g_object_class_list_properties(object_class, &n_pspecs);

	/* Allocate parameters */
	ser_params = g_new0(GsznParameter, n_pspecs);

	/* Iterate and transform every GValue param into a serialized param */
	for (i = 0, j = 0; i < n_pspecs; i++) {
		GParamSpec *pspec = object_pspecs[i];
		GsznParameter *ser_param = &ser_params[j];
		GValue value = G_VALUE_INIT;
		GError *err = NULL;

		/* Perform various checks on property */
		if (!is_property_serialization_wanted(pspec, serialize_flag_only)) {
			g_debug("Not serializing '%s:%s': not flagged as serializable",
			        G_OBJECT_TYPE_NAME(object), pspec->name);
			goto next_iteration;
		}

		if (!is_property_readable(pspec)) {
			g_debug("Not serializing '%s:%s': not readable",
			        G_OBJECT_TYPE_NAME(object), pspec->name);
			goto next_iteration;
		}

		if (is_property_deprecated(pspec)) {
			g_debug("Not serializing '%s:%s': deprecated",
			        G_OBJECT_TYPE_NAME(object), pspec->name);
			goto next_iteration;
		}

		if (!is_property_type_serializable(pspec)) {
			g_debug("Not serializing '%s:%s': type is not serializable",
			        G_OBJECT_TYPE_NAME(object), pspec->name);
			goto next_iteration;
		}

		/* Get property value */
		g_value_init(&value, pspec->value_type);
		g_object_get_property(object, pspec->name, &value);

		/* Check if it's the default value */
		if (value_is_default(&value, pspec) && non_default_only) {
			g_debug("Not serializing '%s:%s': default value",
			        G_OBJECT_TYPE_NAME(object), pspec->name);
			g_value_unset(&value);
			goto next_iteration;
		}

		/* Serialize value */
		gszn_transform_value_to_string(&value, &ser_param->string, &err);
		if (err) {
			g_debug("Failed to transform '%s:%s' to string: %s",
			        G_OBJECT_TYPE_NAME(object), pspec->name, err->message);
			g_clear_error(&err);
			goto next_iteration;
		}

		/* Serialize property name */
		if (tweak_property_name)
			ser_param->name = tweak_property_name(pspec->name);
		else
			ser_param->name = g_strdup(pspec->name);

		/* One more param */
		j++;

	next_iteration:
		/* Free temporary value */
		if (G_VALUE_TYPE(&value))
			g_value_unset(&value);
	}

	/* Cleanup */
	g_free(object_pspecs);

	/* Return NULL if no property have been serialized */
	if (j == 0) {
		g_free(ser_params);
		ser_params = NULL;
	}

	*n_params = j;
	return ser_params;
}

static void
gszn_serializer_serialize_object(GsznSerializer *self, GsznBackend *backend, GObject *object)
{
	GsznSerializerPrivate *priv = self->priv;
	GsznParameter *ser_params;
	guint n_ser_params;
	gchar *object_name;
	gchar *object_uid;
	GsznBackendIter *iter;

	/* Serialize object parameters */
	ser_params = make_serialized_parameters(object, &n_ser_params, priv->serialize_flag_only,
	                                        priv->non_default_only,
	                                        priv->settings->ser_property_name);
	if (ser_params == NULL)
		return;

	/* Serialize object name */
	if (priv->settings->ser_object_name)
		object_name = priv->settings->ser_object_name(G_OBJECT_TYPE_NAME(object));
	else
		object_name = g_strdup(G_OBJECT_TYPE_NAME(object));

	/* Get object uid */
	object_uid = gszn_get_object_uid(object);

	/* Add object */
	iter = gszn_backend_add_object(backend, object_name, object_uid);

	/* Add properties */
	gszn_backend_add_properties(backend, iter, ser_params, n_ser_params);

	/* Cleanup - parameters are consumed by the backend, no need to free */
	gszn_backend_free_iter(backend, iter);
	g_free(object_uid);
	g_free(object_name);
}

/*
 * Object list helpers
 */

static void
on_object_notify(GObject *object, GParamSpec *pspec, GsznSerializer *self)
{
	/* Just emit a signal to say that something happened */
	g_signal_emit(self, signals[SIGNAL_OBJECT_CHANGED], 0, object, pspec);
}

static void
watch_object_all(GObject *object, GCallback cb, gpointer data)
{
	g_signal_connect(object, "notify", cb, data);
}

static void
watch_object_serialize_flag_only(GObject *object, GCallback cb, gpointer data)
{
	GObjectClass *class;
	GParamSpec **pspecs;
	guint n_pspecs;
	guint i;

	/* Get properties list */
	class = G_OBJECT_GET_CLASS(object);
	pspecs = g_object_class_list_properties(class, &n_pspecs);

	/* Connect only to properties flagged with SERIALIZE */
	for (i = 0; i < n_pspecs; i++) {
		GParamSpec *pspec;
		gchar *signal_name;

		pspec = pspecs[i];

		if (!(pspec->flags & GSZN_PARAM_SERIALIZE))
			continue;

		signal_name = g_strconcat("notify::", pspec->name, NULL);
		g_signal_connect(object, signal_name, cb, data);
		g_free(signal_name);
	}

	/* Cleanup */
	g_free(pspecs);
}

static void
unwatch_object(GObject *object, gpointer data)
{
	g_signal_handlers_disconnect_by_data(object, data);
}

/*
 * Methods
 */

void
gszn_serializer_add_object(GsznSerializer *self, GObject *object)
{
	GsznSerializerPrivate *priv = self->priv;
	GList *existing_item;

	/* Check if the object was already added before */
	existing_item = g_list_find(priv->objects, object);
	if (existing_item) {
		g_warning("Object %p already present in internal list", object);
		return;
	}

	/* Watch for property change */
	if (priv->watch) {
		if (priv->serialize_flag_only)
			watch_object_serialize_flag_only(object, G_CALLBACK(on_object_notify),
			                                 self);
		else
			watch_object_all(object, G_CALLBACK(on_object_notify), self);
	}

	/* Add the object to internal list */
	g_object_ref(object);
	priv->objects = g_list_append(priv->objects, object);
}

void
gszn_serializer_remove_object(GsznSerializer *self, GObject *object)
{
	GsznSerializerPrivate *priv = self->priv;
	GList *existing_item;

	/* Check if the object was already added before */
	existing_item = g_list_find(priv->objects, object);
	if (existing_item == NULL) {
		g_warning("Object %p not found in internal list", object);
		return;
	}

	/* Unwatch */
	if (priv->watch)
		unwatch_object(object, self);

	/* Remove from internal list */
	priv->objects = g_list_remove(priv->objects, object);
	g_object_unref(object);
}

void
gszn_serializer_add_list(GsznSerializer *self, GList *objects)
{
	GList *item;

	for (item = objects; item; item = item->next) {
		GObject *object;

		g_assert(G_IS_OBJECT(item->data));
		object = item->data;

		gszn_serializer_add_object(self, object);
	}
}

void
gszn_serializer_remove_list(GsznSerializer *self, GList *objects)
{
	GList *item;

	for (item = objects; item; item = item->next) {
		GObject *object;

		g_assert(G_IS_OBJECT(item->data));
		object = item->data;

		gszn_serializer_remove_object(self, object);
	}
}

gchar *
gszn_serializer_print(GsznSerializer *self)
{
	GsznSerializerPrivate *priv = self->priv;
	GsznBackend *backend;
	GList *item;
	gchar *text;

	/* Create the serialization backend */
	backend = g_object_new(priv->settings->backend_type, NULL);

	/* Serialize objects */
	for (item = priv->objects; item; item = item->next) {
		GObject *object;

		object = item->data;
		gszn_serializer_serialize_object(self, backend, object);
	}

	/* Print */
	text = gszn_backend_print(backend);

	/* Unref */
	g_object_unref(backend);

	return text;
}

GsznSerializer *
gszn_serializer_new(GsznSettings *settings, GsznSerializerFlags flags)
{
	return g_object_new(GSZN_TYPE_SERIALIZER,
	                    "settings", settings,
	                    "flags", flags,
	                    NULL);
}

/*
 * Property accessors
 */

static void
gszn_serializer_get_property(GObject    *object,
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
gszn_serializer_set_property(GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	GsznSerializer *self = GSZN_SERIALIZER(object);
	GsznSerializerPrivate *priv = self->priv;

	switch (property_id) {
	case PROP_SETTINGS:
		priv->settings = g_value_dup_boxed(value);
		break;

	case PROP_FLAGS:
		priv->flags = g_value_get_flags(value);
		priv->serialize_flag_only =
		        priv->flags & GSZN_SERIALIZER_FLAG_SERIALIZE_FLAG_ONLY ?
		        TRUE : FALSE;
		priv->non_default_only =
		        priv->flags & GSZN_SERIALIZER_FLAG_NON_DEFAULT_ONLY ?
		        TRUE : FALSE;
		priv->watch =
		        priv->flags & GSZN_SERIALIZER_FLAG_WATCH ?
		        TRUE : FALSE;
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
gszn_serializer_finalize(GObject *object)
{
	GsznSerializer *self = GSZN_SERIALIZER(object);
	GsznSerializerPrivate *priv = self->priv;
	GList *item;

	/* Release object list */
	for (item = priv->objects; item; item = item->next) {
		GObject *item_object;

		item_object = item->data;

		if (priv->watch)
			unwatch_object(item_object, self);

		g_object_unref(item_object);
	}

	g_list_free(priv->objects);

	/* Free strings */
	gszn_settings_free(priv->settings);

	/* Chain up */
	G_OBJECT_CLASS(gszn_serializer_parent_class)->finalize(object);
}

static void
gszn_serializer_constructed(GObject *object)
{
	GsznSerializer *self = GSZN_SERIALIZER(object);
	GsznSerializerPrivate *priv = self->priv;

	/* Settings are mandatory */
	g_assert(priv->settings != NULL);

	/* Chain up */
	if (G_OBJECT_CLASS(gszn_serializer_parent_class)->constructed)
		G_OBJECT_CLASS(gszn_serializer_parent_class)->constructed(object);
}

static void
gszn_serializer_init(GsznSerializer *self)
{
	/* Initialize private pointer */
	self->priv = gszn_serializer_get_instance_private(self);
}

static void
gszn_serializer_class_init(GsznSerializerClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	/* Override GObject methods */
	object_class->finalize = gszn_serializer_finalize;
	object_class->constructed = gszn_serializer_constructed;

	/* Properties */
	object_class->get_property = gszn_serializer_get_property;
	object_class->set_property = gszn_serializer_set_property;

	properties[PROP_SETTINGS] =
	        g_param_spec_boxed("settings", "Settings", NULL,
	                           GSZN_TYPE_SETTINGS,
	                           G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE |
	                           G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_FLAGS] =
	        g_param_spec_flags("flags", "Flags", NULL,
	                           GSZN_SERIALIZER_FLAGS_ENUM_TYPE,
	                           GSZN_SERIALIZER_FLAG_DEFAULT,
	                           G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE |
	                           G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties(object_class, PROP_N, properties);

	/* Signals */
	signals[SIGNAL_OBJECT_CHANGED] =
	        g_signal_new("object-changed", G_TYPE_FROM_CLASS(class),
	                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                     0, NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE,
	                     2, G_TYPE_OBJECT, G_TYPE_PARAM);
}
