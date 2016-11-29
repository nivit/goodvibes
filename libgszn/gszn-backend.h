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

#ifndef __LIBGSZN_GSZN_BACKEND_H__
#define __LIBGSZN_GSZN_BACKEND_H__

#include <glib-object.h>

/* GObject declarations */

#define GSZN_TYPE_BACKEND gszn_backend_get_type()

G_DECLARE_INTERFACE(GsznBackend, gszn_backend, GSZN, BACKEND, GObject)

/* Data types */

typedef struct _GsznBackendIter GsznBackendIter;

#define GSZN_BACKEND_ITER(i) ((GsznBackendIter *)(i))

struct _GsznParameter {
	gchar *name;
	gchar *string;
};

typedef struct _GsznParameter GsznParameter;

struct _GsznBackendInterface {
	/* Parent interface */
	GTypeInterface parent_iface;

	/* Methods */
	gchar           *(*print)                 (GsznBackend *self);
	gboolean         (*parse)                 (GsznBackend *self, const gchar *data,
	                                           GError **err);

	void             (*free_iter)             (GsznBackend *self, GsznBackendIter *iter);
	GsznBackendIter *(*get_uninitialized_iter)(GsznBackend *self);
	gboolean         (*loop)                  (GsznBackend *self, GsznBackendIter *iter,
	                                           const gchar **object_name);

	GsznBackendIter *(*get_object)            (GsznBackend *self, const gchar *object_name,
	                                           const gchar *object_uid);
	GsznBackendIter *(*add_object)            (GsznBackend *self, const gchar *object_name,
	                                           const gchar *object_uid);

	const gchar     *(*get_object_uid)        (GsznBackend *self, GsznBackendIter *iter);
	GsznParameter   *(*get_properties)        (GsznBackend *self, GsznBackendIter *iter,
	                                           guint *n_params);
	void             (*add_properties)        (GsznBackend *self, GsznBackendIter *iter,
	                                           GsznParameter *params, guint n_params);
};

/* Methods */

gchar           *gszn_backend_print(GsznBackend *self);
gboolean         gszn_backend_parse(GsznBackend *self, const gchar *data, GError **err);

void             gszn_backend_free_iter(GsznBackend *self, GsznBackendIter *iter);
GsznBackendIter *gszn_backend_get_uninitialized_iter(GsznBackend *self);
gboolean         gszn_backend_loop     (GsznBackend *self, GsznBackendIter *iter,
                                        const gchar **object_name);

GsznBackendIter *gszn_backend_get_object    (GsznBackend *self, const gchar *object_name,
                                             const gchar *object_uid);
GsznBackendIter *gszn_backend_add_object    (GsznBackend *self, const gchar *object_name,
                                             const gchar *object_uid);

const gchar     *gszn_backend_get_object_uid(GsznBackend *self, GsznBackendIter *iter);
GsznParameter   *gszn_backend_get_properties(GsznBackend *self, GsznBackendIter *iter,
                                             guint *n_params);
void             gszn_backend_add_properties(GsznBackend *self, GsznBackendIter *iter,
                                             GsznParameter *params, guint n_params);

#endif /* __LIBGSZN_GSZN_BACKEND_H__ */
