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

#include <string.h>
#include <glib.h>
#include <glib-object.h>

#include "libgszn/gszn.h"

#include "additions/glib.h"
#include "additions/glib-object.h"

#include "framework/log.h"
#include "framework/ock-framework.h"

#include "core/ock-conf.h"

// TODO Set SAVE_DELAY to something way longer, like 10 seconds.
//      Then, test early exit to be sure everything is saved.
#define SAVE_DELAY 1

/*
 * GObject definitions
 */

struct _OckConfPrivate {
	/* Load/save pathes */
	GSList *load_pathes;
	gchar  *save_path;
	/* Timeout id, > 0 if a save operation is scheduled */
	guint   save_timeout_id;
	/* Serialization objects */
	GsznSettings     *serialization_settings;
	GsznSerializer   *serializer;
	GsznDeserializer *deserializer;
};

typedef struct _OckConfPrivate OckConfPrivate;

struct _OckConf {
	/* Parent instance structure */
	GObject         parent_instance;
	/* Private data */
	OckConfPrivate *priv;
};

G_DEFINE_TYPE_WITH_CODE(OckConf, ock_conf, G_TYPE_OBJECT,
                        G_ADD_PRIVATE(OckConf)
                        G_IMPLEMENT_INTERFACE(OCK_TYPE_ERRORABLE,
                                        ock_errorable_dummy_interface_init))

/*
 * Serialization settings
 */

static gchar *
serialize_object_name(const gchar *object_type_name)
{
	if (g_str_has_prefix(object_type_name, "Ock"))
		object_type_name += 3;

	return g_strdup(object_type_name);
}

static gchar *
deserialize_object_name(const gchar *object_name)
{
	return g_strconcat("Ock", object_name, NULL);
}

static gchar *
serialize_property_name(const gchar *src)
{
	/*
	 * Convert property name case to camelcase.
	 * We assume that the input contains only lowercase, '-' and '_'.
	 *
	 *     'a-property-name' > 'APropertyName'
	 */

	gchar *camel;
	guint i, j;
	guint src_len;

	if (src == NULL)
		return NULL;

	src_len = strlen(src);
	camel = g_malloc(src_len + 1);

	for (i = 0, j = 0; src[j] != '\0'; i++, j++) {
		gboolean uppercase = FALSE;

		/* First char uppercased */
		if (i == 0)
			uppercase = TRUE;

		/* Dash or underscore removed, next char uppercased */
		while (src[j] == '-' || src[j] == '_') {
			uppercase = TRUE;
			j++;
		}

		if (uppercase)
			camel[i] = g_ascii_toupper(src[j]);
		else
			camel[i] = src[j];
	}

	camel[i] = '\0';
	camel = g_realloc(camel, i);

	return camel;
}

static gchar *
deserialize_property_name(const gchar *src)
{
	/*
	 * Convert camelcase to property name case.
	 * We assume that the input contains only lowercase and uppercase.
	 *
	 *     'APropertyName' > 'a-property-name'
	 */

	gchar *dst;
	guint i, j;
	guint src_len, dst_len;

	if (src == NULL)
		return NULL;

	/* We assume there won't be more than 8 dashes to add */
	src_len = strlen(src);
	dst_len = src_len + 8;
	dst = g_malloc(dst_len + 1);

	for (i = 0, j = 0; src[j] != '\0' && i < dst_len; i++, j++) {
		if (i == 0) {
			/* First char lowercased */
			dst[i] = g_ascii_tolower(src[j]);
		} else if (g_ascii_isupper(src[j])) {
			/* Uppercased are prepended by a dash and lowercased */
			dst[i++] = '-';
			dst[i] = g_ascii_tolower(src[j]);
		} else {
			/* Just copy */
			dst[i] = src[j];
		}
	}

	dst[i] = '\0';
	dst = g_realloc(dst, i);

	return dst;
}

/*
 * Signal handlers
 */

static gboolean
when_save_timeout(gpointer data)
{
	OckConf *self = OCK_CONF(data);
	OckConfPrivate *priv = self->priv;

	ock_conf_save(self);

	priv->save_timeout_id = 0;

	return G_SOURCE_REMOVE;
}

static void
ock_conf_schedule_save(OckConf *self)
{
	OckConfPrivate *priv = self->priv;

	if (priv->save_timeout_id > 0)
		g_source_remove(priv->save_timeout_id);

	priv->save_timeout_id =
	        g_timeout_add_seconds(SAVE_DELAY, when_save_timeout, self);
}

static void
on_serializer_object_changed(GsznSerializer *serializer G_GNUC_UNUSED,
                             GObject *object G_GNUC_UNUSED,
                             GParamSpec *pspec G_GNUC_UNUSED,
                             OckConf *self)
{
	ock_conf_schedule_save(self);
}

/* Stuff */

void
ock_conf_unwatch(OckConf *self)
{
	OckConfPrivate *priv = self->priv;
	GsznSerializer *serializer = priv->serializer;

	/* This should be called only once at cleanup */
	g_assert(serializer);

	/* Run any pending save operation */
	if (priv->save_timeout_id > 0)
		when_save_timeout(self);

	/* Dispose of serializer */
	g_signal_handlers_disconnect_by_data(serializer, self);
	g_object_unref(serializer);
	priv->serializer = NULL;
}

void
ock_conf_watch(OckConf *self)
{
	OckConfPrivate *priv = self->priv;
	GsznSerializer *serializer;

	TRACE("%p", self);

	/* Create a serializer object */
	serializer = gszn_serializer_new(priv->serialization_settings,
	                                 GSZN_SERIALIZER_FLAG_SERIALIZE_FLAG_ONLY |
	                                 GSZN_SERIALIZER_FLAG_NON_DEFAULT_ONLY |
	                                 GSZN_SERIALIZER_FLAG_WATCH);

	/* Add the configurable objects to the serializer */
	gszn_serializer_add_list(serializer, ock_framework_configurable_list);

	/* Watch for object changes */
	g_signal_connect(serializer, "object-changed",
	                 G_CALLBACK(on_serializer_object_changed), self);

	/* This should be called only once at startup */
	g_assert(priv->serializer == NULL);
	priv->serializer = serializer;
}

void
ock_conf_apply(OckConf *self)
{
	OckConfPrivate *priv = self->priv;
	GsznDeserializer *deserializer = priv->deserializer;

	TRACE("%p", self);

	/* If there was no configuration file, there's no deserializer */
	if (deserializer == NULL)
		return;

	/* Restore objects state from serialization data */
	gszn_deserializer_restore_all(deserializer, ock_framework_configurable_list);

	/* Dispose of deserializer */
	g_object_unref(deserializer);
	priv->deserializer = NULL;
}

void
ock_conf_save(OckConf *self)
{
	OckConfPrivate *priv = self->priv;
	GsznSerializer *serializer = priv->serializer;
	GError *err = NULL;
	gchar *text;

	/* Stringify data */
	g_assert(serializer);
	text = gszn_serializer_print(serializer);

	/* Write to file */
	ock_file_write_sync(priv->save_path, text, &err);
	g_free(text);

	/* Handle error */
	if (err == NULL) {
		INFO("Configuration saved to '%s'", priv->save_path);
	} else {
		gchar *str;

		str = g_strdup_printf("Failed to save configuration: %s", err->message);

		INFO("%s", str);
		ock_errorable_emit_error(OCK_ERRORABLE(self), str);

		g_free(str);
		g_clear_error(&err);
	}
}

void
ock_conf_load(OckConf *self)
{
	OckConfPrivate *priv = self->priv;
	GsznDeserializer *deserializer;
	GSList *item = NULL;

	TRACE("%p", self);

	/* This should be called only once at startup */
	g_assert(priv->serializer == NULL);
	g_assert(priv->deserializer == NULL);

	/* Create the deserializer */
	deserializer = gszn_deserializer_new(priv->serialization_settings);

	/* Load from a list of pathes */
	for (item = priv->load_pathes; item; item = item->next) {
		GError *err = NULL;
		const gchar *path = item->data;
		gchar *text;

		/* Attempt to read file */
		ock_file_read_sync(path, &text, &err);
		if (err) {
			WARNING("%s", err->message);
			g_clear_error(&err);
			continue;
		}

		/* Attempt to parse it */
		gszn_deserializer_parse(deserializer, text, &err);
		g_free(text);
		if (err) {
			WARNING("Failed to parse '%s': %s", path, err->message);
			g_clear_error(&err);
			continue;
		}

		/* Success */
		break;
	}

	/* Check if we got something */
	if (item) {
		const gchar *loaded_path = item->data;

		INFO("Configuration loaded from file '%s'", loaded_path);
		priv->deserializer = deserializer;
	} else {
		INFO("No configuration file found, using default");
		g_object_unref(deserializer);
	}
}

OckConf *
ock_conf_new(void)
{
	return g_object_new(OCK_TYPE_CONF, NULL);
}

/*
 * GObject methods
 */

static void
ock_conf_finalize(GObject *object)
{
	OckConf *self = OCK_CONF(object);
	OckConfPrivate *priv = self->priv;

	TRACE("%p", object);

	/* Ensure that everything happened in order */
	g_assert(priv->save_timeout_id == 0);
	g_assert(priv->serializer == NULL);
	g_assert(priv->deserializer == NULL);

	/* Free serialization settings */
	gszn_settings_free(priv->serialization_settings);

	/* Free pathes */
	g_free(priv->save_path);
	g_slist_free_full(priv->load_pathes, g_free);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_conf, object);
}


static void
ock_conf_constructed(GObject *object)
{
	OckConf *self = OCK_CONF(object);
	OckConfPrivate *priv = self->priv;
	GsznSettings *settings;

	TRACE("%p");

	/* Initialize pathes */
	priv->load_pathes = ock_get_existing_path_list
	                    (OCK_DIR_USER_CONFIG | OCK_DIR_SYSTEM_CONFIG, "config");
	priv->save_path = g_build_filename(ock_get_user_config_dir(), "config", NULL);

	/* Create serialization settings */
	settings = gszn_settings_new();
	settings->backend_type        = GSZN_TYPE_BACKEND_KEYFILE;
	settings->title               = "Configuration";
	settings->ser_object_name     = serialize_object_name;
	settings->deser_object_name   = deserialize_object_name;
	settings->ser_property_name   = serialize_property_name;
	settings->deser_property_name = deserialize_property_name;
	priv->serialization_settings  = settings;

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_conf, object);
}

static void
ock_conf_init(OckConf *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_conf_get_instance_private(self);
}

static void
ock_conf_class_init(OckConfClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_conf_finalize;
	object_class->constructed = ock_conf_constructed;
}
