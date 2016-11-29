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
 * GObject definitions
 */

G_DEFINE_INTERFACE(GsznBackend, gszn_backend, G_TYPE_OBJECT)

/* Dummy structure to ensure type checking at compile-time.
 * A bit better than using a void* pointer, I guess.
 */
struct _GsznBackendIter {};

/* Methods */

const gchar *
gszn_backend_get_object_uid(GsznBackend *self, GsznBackendIter *iter)
{
	GsznBackendInterface *iface;

	g_assert(GSZN_IS_BACKEND(self));
	iface = GSZN_BACKEND_GET_IFACE(self);

	g_assert(iface->get_object_uid != NULL);
	return iface->get_object_uid(self, iter);
}

GsznParameter *
gszn_backend_get_properties(GsznBackend *self, GsznBackendIter *iter,
                            guint *n_params)
{
	GsznBackendInterface *iface;

	g_assert(GSZN_IS_BACKEND(self));
	iface = GSZN_BACKEND_GET_IFACE(self);

	g_assert(iface->get_properties != NULL);
	return iface->get_properties(self, iter, n_params);
}

void
gszn_backend_add_properties(GsznBackend *self, GsznBackendIter *iter,
                            GsznParameter *params, guint n_params)
{
	GsznBackendInterface *iface;

	g_assert(GSZN_IS_BACKEND(self));
	iface = GSZN_BACKEND_GET_IFACE(self);

	g_assert(iface->add_properties != NULL);
	iface->add_properties(self, iter, params, n_params);
}

GsznBackendIter *
gszn_backend_get_object(GsznBackend *self, const gchar *object_name, const gchar *object_uid)
{
	GsznBackendInterface *iface;

	g_assert(GSZN_IS_BACKEND(self));
	iface = GSZN_BACKEND_GET_IFACE(self);

	g_assert(iface->get_object != NULL);
	return iface->get_object(self, object_name, object_uid);
}

GsznBackendIter *
gszn_backend_add_object(GsznBackend *self, const gchar *object_name, const gchar *object_uid)
{
	GsznBackendInterface *iface;

	g_assert(GSZN_IS_BACKEND(self));
	iface = GSZN_BACKEND_GET_IFACE(self);

	g_assert(iface->add_object != NULL);
	return iface->add_object(self, object_name, object_uid);
}

gboolean
gszn_backend_loop(GsznBackend *self, GsznBackendIter *iter, const gchar **object_name)
{
	GsznBackendInterface *iface;

	g_assert(GSZN_IS_BACKEND(self));
	iface = GSZN_BACKEND_GET_IFACE(self);

	g_assert(iface->loop != NULL);
	return iface->loop(self, iter, object_name);
}

GsznBackendIter *
gszn_backend_get_uninitialized_iter(GsznBackend *self)
{
	GsznBackendInterface *iface;

	g_assert(GSZN_IS_BACKEND(self));
	iface = GSZN_BACKEND_GET_IFACE(self);

	g_assert(iface->get_uninitialized_iter != NULL);
	return iface->get_uninitialized_iter(self);
}

void
gszn_backend_free_iter(GsznBackend *self, GsznBackendIter *iter)
{
	GsznBackendInterface *iface;

	g_assert(GSZN_IS_BACKEND(self));
	iface = GSZN_BACKEND_GET_IFACE(self);

	g_assert(iface->free_iter != NULL);
	iface->free_iter(self, iter);
}

gboolean
gszn_backend_parse(GsznBackend *self, const gchar *data, GError **err)
{
	GsznBackendInterface *iface;

	g_assert(GSZN_IS_BACKEND(self));
	iface = GSZN_BACKEND_GET_IFACE(self);

	g_assert(iface->parse != NULL);
	return iface->parse(self, data, err);
}

gchar *
gszn_backend_print(GsznBackend *self)
{
	GsznBackendInterface *iface;

	g_assert(GSZN_IS_BACKEND(self));
	iface = GSZN_BACKEND_GET_IFACE(self);

	g_assert(iface->print != NULL);
	return iface->print(self);
}

/*
 * GObject methods
 */

static void
gszn_backend_default_init(GsznBackendInterface *iface G_GNUC_UNUSED)
{
}
