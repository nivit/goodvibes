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

#include "libgszn/gszn-error.h"
#include "libgszn/gszn-backend.h"
#include "libgszn/gszn-backend-keyfile.h"

/*
 * GObject definitions
 */

struct _GsznBackendKeyfilePrivate {
	GKeyFile *keyfile;
};

typedef struct _GsznBackendKeyfilePrivate GsznBackendKeyfilePrivate;

struct _GsznBackendKeyfile {
	/* Parent instance structure */
	GObject parent_instance;
	/* Private data */
	GsznBackendKeyfilePrivate *priv;
};

static void gszn_backend_keyfile_interface_init(GsznBackendInterface *iface);

G_DEFINE_TYPE_WITH_CODE(GsznBackendKeyfile, gszn_backend_keyfile, G_TYPE_OBJECT,
                        G_ADD_PRIVATE(GsznBackendKeyfile)
                        G_IMPLEMENT_INTERFACE(GSZN_TYPE_BACKEND,
                                        gszn_backend_keyfile_interface_init))

/*
 * GsznBackend interface methods
 */

struct _GsznBackendKeyfileIter {
	/* List of groups */
	gchar **groups;
	gsize n_groups;
	/* Ptr to the current group */
	gchar **cur;
	/* Current type, if any */
	gchar *type;
	/* Current object identifiers */
	const gchar *object_name;
	const gchar *object_uid;

};

typedef struct _GsznBackendKeyfileIter GsznBackendKeyfileIter;

#define GSZN_BACKEND_KEYFILE_ITER(i) ((GsznBackendKeyfileIter *)(i))

static GsznBackendIter *
gszn_backend_keyfile_get_uninitialized_iter(GsznBackend *backend)
{
	GsznBackendKeyfile *self = GSZN_BACKEND_KEYFILE(backend);
	GsznBackendKeyfilePrivate *priv = self->priv;
	GsznBackendKeyfileIter *iter;

	iter = g_new0(GsznBackendKeyfileIter, 1);
	iter->groups = g_key_file_get_groups(priv->keyfile, &iter->n_groups);

	return GSZN_BACKEND_ITER(iter);
}

static void
gszn_backend_keyfile_free_iter(GsznBackend *backend G_GNUC_UNUSED, GsznBackendIter *backend_iter)
{
	GsznBackendKeyfileIter *iter = GSZN_BACKEND_KEYFILE_ITER(backend_iter);

	g_free(iter->type);
	g_strfreev(iter->groups);
	g_free(iter);
}

static gboolean
gszn_backend_keyfile_loop(GsznBackend *backend, GsznBackendIter *backend_iter,
                          const gchar **object_name)
{
	GsznBackendKeyfile *self = GSZN_BACKEND_KEYFILE(backend);
	GsznBackendKeyfilePrivate *priv = self->priv;
	GsznBackendKeyfileIter *iter = GSZN_BACKEND_KEYFILE_ITER(backend_iter);
	const gchar *group;

	/* Init output parameters */
	if (object_name)
		*object_name = NULL;

	/* Cleanup iter first */
	g_free(iter->type);
	iter->type = NULL;

	/* Nothing if no groups */
	if (iter->groups == NULL)
		return FALSE;

	/* Increment iter */
	if (iter->cur == NULL)
		iter->cur = iter->groups;
	else
		iter->cur++;

	/* Check if we reached the end */
	group = *(iter->cur);
	if (group == NULL)
		return FALSE;

	/* Find the object type name, which is:
	 * - the special field 'type', if any
	 * - the group name otherwise
	 */
	iter->type = g_key_file_get_string(priv->keyfile, group, "type", NULL);

	if (iter->type) {
		iter->object_name = iter->type;
		iter->object_uid = group;
	} else {
		iter->object_name = group;
		iter->object_uid = NULL;
	}

	if (object_name)
		*object_name = iter->object_name;

	return TRUE;
}

const gchar *
gszn_backend_keyfile_get_object_uid(GsznBackend *self G_GNUC_UNUSED, GsznBackendIter *backend_iter)
{
	GsznBackendKeyfileIter *iter = GSZN_BACKEND_KEYFILE_ITER(backend_iter);

	return iter->object_uid;
}

static GsznParameter *
gszn_backend_keyfile_get_properties(GsznBackend *backend G_GNUC_UNUSED,
                                    GsznBackendIter *backend_iter,
                                    guint *n_params)
{
	GsznBackendKeyfile *self = GSZN_BACKEND_KEYFILE(backend);
	GsznBackendKeyfilePrivate *priv = self->priv;
	GsznBackendKeyfileIter *iter = GSZN_BACKEND_KEYFILE_ITER(backend_iter);
	GsznParameter *params;
	const gchar *group;
	gchar **keys;
	gsize n_keys;
	guint i, j;

	g_return_val_if_fail(n_params != NULL, NULL);
	*n_params = 0;

	group = *(iter->cur);
	if (group == NULL)
		return NULL;

	keys = g_key_file_get_keys(priv->keyfile, group, &n_keys, NULL);
	if (keys == NULL)
		return NULL;

	params = g_new0(GsznParameter, n_keys);

	for (i = 0, j = 0; i < n_keys; i++) {
		GsznParameter *param = &params[j];
		const gchar *key = keys[i];

		if (!g_strcmp0(key, "type"))
			continue;

		param->name = g_strdup(key);
		param->string = g_key_file_get_value(priv->keyfile, group, key, NULL);

		j++;
	}

	g_strfreev(keys);

	*n_params = j;
	return params;
}

static void
gszn_backend_keyfile_add_properties(GsznBackend *backend G_GNUC_UNUSED,
                                    GsznBackendIter *backend_iter,
                                    GsznParameter *params, guint n_params)
{
	GsznBackendKeyfile *self = GSZN_BACKEND_KEYFILE(backend);
	GsznBackendKeyfilePrivate *priv = self->priv;
	GsznBackendKeyfileIter *iter = GSZN_BACKEND_KEYFILE_ITER(backend_iter);
	const gchar *group;
	guint i;

	group = *(iter->cur);
	if (group == NULL)
		return;

	for (i = 0; i < n_params; i++) {
		GsznParameter *param = &params[i];

		g_key_file_set_value(priv->keyfile, group, param->name, param->string);
		g_free(param->name);
		g_free(param->string);
	}

	g_free(params);
}

static GsznBackendIter *
gszn_backend_keyfile_get_object(GsznBackend *backend, const gchar *object_name,
                                const gchar *object_uid)
{
	GsznBackendIter *backend_iter;

	backend_iter = gszn_backend_keyfile_get_uninitialized_iter(backend);

	while (gszn_backend_keyfile_loop(backend, backend_iter, NULL)) {
		GsznBackendKeyfileIter *iter = GSZN_BACKEND_KEYFILE_ITER(backend_iter);

		if (!g_strcmp0(object_name, iter->object_name) &&
		    !g_strcmp0(object_uid, iter->object_uid))
			return backend_iter;
	}

	gszn_backend_keyfile_free_iter(backend, backend_iter);

	return NULL;
}

static GsznBackendIter *
gszn_backend_keyfile_add_object(GsznBackend *backend, const gchar *object_name,
                                const gchar *object_uid)
{
	GsznBackendKeyfile *self = GSZN_BACKEND_KEYFILE(backend);
	GsznBackendKeyfilePrivate *priv = self->priv;
	GsznBackendKeyfileIter *iter;
	GsznBackendIter *backend_iter;

	/* Keyfile doesn't support multiple objects with same name and same uid.
	 * However this function is not supposed to fail. So in case user is
	 * trying that, we just return an iter to an existing object instead
	 * of creating a new one.
	 */
	backend_iter = gszn_backend_keyfile_get_object(backend, object_name, object_uid);
	if (backend_iter)
		return backend_iter;

	/* Time to 'add' this new group.
	 * It's a bit tricky since keyfile doesn't support adding a group,
	 * only adding values. Groups addition and deletion is automatically
	 * handled by the keyfile, which is usually practical, but here
	 * it's a real pain...
	 */

	/* First situation, we have an object uid. This is the easy one, since
	 * object uid forces us to add a value to the group, so we don't have to
	 * play smart.
	 */
	if (object_uid) {
		/* Add the new object */
		g_key_file_set_value(priv->keyfile, object_uid, "type", object_name);

		/* Recursive call */
		return gszn_backend_keyfile_get_object(backend, object_name, object_uid);
	}

	/* Second situation, we have no values for this object, we can't add it to the keyfile.
	 * So we workaround that by adding a group within the iterator,
	 * so that we can still return a valid iterator.
	 * The group will be created in the keyfile later on, when setting values.
	 */
	backend_iter = gszn_backend_keyfile_get_uninitialized_iter(backend);
	iter = GSZN_BACKEND_KEYFILE_ITER(backend_iter);

	iter->groups = g_realloc(iter->groups, (iter->n_groups + 2) * sizeof(gchar *));
	iter->groups[iter->n_groups] = g_strdup(object_name);
	iter->groups[iter->n_groups + 1] = NULL;
	iter->n_groups++;

	iter->cur = &iter->groups[iter->n_groups - 1];
	iter->object_name = *(iter->cur);
	iter->object_uid = NULL;

	return GSZN_BACKEND_ITER(iter);
}

static gchar *
gszn_backend_keyfile_print(GsznBackend *backend)
{
	GsznBackendKeyfile *self = GSZN_BACKEND_KEYFILE(backend);
	GsznBackendKeyfilePrivate *priv = self->priv;
	GKeyFile *keyfile = priv->keyfile;

	/* According to the doc, this function never returns an error */
	return g_key_file_to_data(keyfile, NULL, NULL);
}

static gboolean
gszn_backend_keyfile_parse(GsznBackend *backend, const gchar *text, GError **err)
{
	GsznBackendKeyfile *self = GSZN_BACKEND_KEYFILE(backend);
	GsznBackendKeyfilePrivate *priv = self->priv;
	GKeyFile *keyfile;
	GError *errtmp = NULL;
	gboolean loaded;

	g_return_val_if_fail(text != NULL, FALSE);
	g_return_val_if_fail(err == NULL || *err == NULL, FALSE);

	keyfile = g_key_file_new();
	loaded = g_key_file_load_from_data(keyfile, text, (gsize) (-1), 0, &errtmp);

	if (loaded == TRUE) {
		g_key_file_unref(priv->keyfile);
		priv->keyfile = keyfile;
	} else if (errtmp) {
		g_set_error_literal(err, GSZN_ERROR, GSZN_ERROR_PARSE,
		                    errtmp->message);
		g_clear_error(&errtmp);
	}

	return loaded;
}

static void
gszn_backend_keyfile_interface_init(GsznBackendInterface *iface)
{
	iface->print = gszn_backend_keyfile_print;
	iface->parse = gszn_backend_keyfile_parse;

	iface->free_iter              = gszn_backend_keyfile_free_iter;
	iface->get_uninitialized_iter = gszn_backend_keyfile_get_uninitialized_iter;
	iface->loop                   = gszn_backend_keyfile_loop;

	iface->get_object     = gszn_backend_keyfile_get_object;
	iface->add_object     = gszn_backend_keyfile_add_object;
	iface->get_object_uid = gszn_backend_keyfile_get_object_uid;
	iface->get_properties = gszn_backend_keyfile_get_properties;
	iface->add_properties = gszn_backend_keyfile_add_properties;
}

/*
 * GObject methods
 */

static void
gszn_backend_keyfile_finalize(GObject *object)
{
	GsznBackendKeyfile *self = GSZN_BACKEND_KEYFILE(object);
	GsznBackendKeyfilePrivate *priv = self->priv;

	/* Free resources */
	g_key_file_unref(priv->keyfile);

	/* Chain up */
	G_OBJECT_CLASS(gszn_backend_keyfile_parent_class)->finalize(object);
}

static void
gszn_backend_keyfile_constructed(GObject *object)
{
	GsznBackendKeyfile *self = GSZN_BACKEND_KEYFILE(object);
	GsznBackendKeyfilePrivate *priv = self->priv;

	/* Create keyfile */
	priv->keyfile = g_key_file_new();

	/* Chain up */
	if (G_OBJECT_CLASS(gszn_backend_keyfile_parent_class)->constructed)
		G_OBJECT_CLASS(gszn_backend_keyfile_parent_class)->constructed(object);
}

static void
gszn_backend_keyfile_init(GsznBackendKeyfile *self)
{
	/* Initialize private pointer */
	self->priv = gszn_backend_keyfile_get_instance_private(self);
}

static void
gszn_backend_keyfile_class_init(GsznBackendKeyfileClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	/* Override GObject methods */
	object_class->finalize = gszn_backend_keyfile_finalize;
	object_class->constructed = gszn_backend_keyfile_constructed;
}
