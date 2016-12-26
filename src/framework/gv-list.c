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

#include "framework/log.h"
#include "framework/gv-list.h"
#include "framework/gv-param-specs.h"


/*
 * List implementation as an object, basically a list
 * of objects, that emit signal when modified, and that needs
 * iterators to be accessed.
 * BE AWARE that this is not a container, in the sense that it
 * doesn't take ownership of the objects when they're added to the list,
 * nor does it unref when they're removed, or when the list is finalized.
 * In other words, there's no equivalent for g_list_free_full().
 */

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Properties */
	PROP_ITEM_TYPE,
	PROP_LENGTH,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * Signals
 */

enum {
	SIGNAL_ITEM_ADDED,
	SIGNAL_ITEM_REMOVED,
	SIGNAL_N
};

static guint signals[SIGNAL_N];

/*
 * GObject definitions
 */

struct _GvListPrivate {
	GType  item_type;
	GList *list;
};

typedef struct _GvListPrivate GvListPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(GvList, gv_list, G_TYPE_OBJECT)

/*
 * Helpers
 */

/*
 * Iterator implementation
 */

struct _GvListIter {
	GList *head, *item;
};

GvListIter *
gv_list_iter_new(GvList *self)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);
	GList *list = priv->list;
	GvListIter *iter;

	iter = g_new0(GvListIter, 1);
	iter->head = g_list_copy_deep(list, (GCopyFunc) g_object_ref, NULL);
	iter->item = iter->head;

	return iter;
}

void
gv_list_iter_free(GvListIter *iter)
{
	g_assert(iter != NULL);

	g_list_free_full(iter->head, g_object_unref);
	g_free(iter);
}

gboolean
gv_list_iter_loop(GvListIter *iter, GObject **object)
{
	g_assert(iter != NULL);
	g_assert(object != NULL);

	*object = NULL;

	if (iter->item == NULL)
		return FALSE;

	*object = iter->item->data;
	iter->item = iter->item->next;

	return TRUE;
}

/*
 * Signal handlers & callbacks
 */

/*
 * Public methods
 */

void
gv_list_foreach_disconnect_by_data(GvList *self, gpointer data)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);
	GList *item;

	for (item = priv->list; item; item = item->next) {
		GObject *obj;

		obj = item->data;
		g_signal_handlers_disconnect_by_data(obj, data);
	}
}

void
gv_list_foreach_connect(GvList *self, const gchar *signal_name,
                        GCallback callback, gpointer data)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);
	GList *item;

	for (item = priv->list; item; item = item->next) {
		GObject *obj;

		obj = item->data;
		g_signal_connect(obj, signal_name, callback, data);
	}
}

void
gv_list_append_object(GvList *self, GObject *item)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);

	g_assert(g_type_is_a(G_OBJECT_TYPE(item), priv->item_type));

	priv->list = g_list_append(priv->list, item);
	g_signal_emit(self, signals[SIGNAL_ITEM_ADDED], 0, item);
}

void
gv_list_remove_object(GvList *self, GObject *item)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);
	GList *llink;

	g_assert(g_type_is_a(G_OBJECT_TYPE(item), priv->item_type));

	llink = g_list_find(priv->list, item);
	if (llink == NULL)
		return;

	priv->list = g_list_delete_link(priv->list, llink);
	g_signal_emit(self, signals[SIGNAL_ITEM_REMOVED], 0, item);
}

GObject *
gv_list_find_by_property_string(GvList *self, const gchar *property_name,
                                const gchar *value_to_match)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);
	GList *item;

	for (item = priv->list; item; item = item->next) {
		GObject *obj;
		gchar *str;

		obj = item->data;
		g_object_get(obj, property_name, &str, NULL);

		if (!g_strcmp0(str, value_to_match)) {
			g_free(str);
			return obj;
		}

		g_free(str);
	}

	return NULL;
}

GObject *
gv_list_find_by_type_name(GvList *self, const gchar *type_name)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);
	GList *item;

	for (item = priv->list; item; item = item->next) {
		GObject *obj;

		obj = item->data;
		if (!g_strcmp0(type_name, G_OBJECT_TYPE_NAME(obj)))
			return obj;
	}

	return NULL;
}

GObject *
gv_list_find(GvList *self, GObject *item_to_find)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);
	GList *item;

	g_assert(g_type_is_a(G_OBJECT_TYPE(item_to_find), priv->item_type));

	item = g_list_find(priv->list, item_to_find);

	return item ? item->data : NULL;
}

gboolean
gv_list_is_empty(GvList *self)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);

	return priv->list == NULL;
}

gchar *
gv_list_print(GvList *self)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);
	GString *str;
	GList *item;

	/* Get ids of every objects contained in the list */
	str = NULL;
	for (item = priv->list; item; item = item->next) {
		GObject *obj;
		const gchar *obj_name;

		obj = item->data;
		obj_name = G_OBJECT_TYPE_NAME(obj);

		if (str == NULL)
			str = g_string_new(obj_name);
		else
			g_string_append_printf(str, ", %s", obj_name);
	}

	return str ? g_string_free(str, FALSE) : NULL;
}

GList *
gv_list_peek(GvList *self)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);

	return priv->list;
}

GvList *
gv_list_new(GType item_type)
{
	return g_object_new(GV_TYPE_LIST, "item-type", item_type, NULL);
}

/*
 * Property accessors
 */

GType
gv_list_get_item_type(GvList *self)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);

	return priv->item_type;
}

static void
gv_list_set_item_type(GvList *self, GType item_type)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);

	priv->item_type = item_type;
}

guint
gv_list_get_length(GvList *self)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);

	return g_list_length(priv->list);
}

static void
gv_list_get_property(GObject     *object,
                     guint       property_id,
                     GValue     *value,
                     GParamSpec *pspec)
{
	GvList *self = GV_LIST(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_ITEM_TYPE:
		g_value_set_gtype(value, gv_list_get_item_type(self));
		break;
	case PROP_LENGTH:
		g_value_set_uint(value, gv_list_get_length(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
gv_list_set_property(GObject       *object,
                     guint         property_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
	GvList *self = GV_LIST(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_ITEM_TYPE:
		gv_list_set_item_type(self, g_value_get_gtype(value));
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
gv_list_finalize(GObject *object)
{
	GvList *self = GV_LIST(object);
	GvListPrivate *priv = gv_list_get_instance_private(self);

	TRACE("%p", object);

	/* The list user is supposed to take care of emptying the list
	 * before emptying it. If it's not the case, emit a warning.
	 */
	if (priv->list != NULL)
		WARNING("Finalizing a non-empty list, memory might be leaked");

	g_list_free(priv->list);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(gv_list, object);
}

static void
gv_list_init(GvList *self)
{
	GvListPrivate *priv = gv_list_get_instance_private(self);

	TRACE("%p", self);

	/* Initialize properties */
	priv->list = NULL;
}

static void
gv_list_class_init(GvListClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = gv_list_finalize;

	/* Properties */
	object_class->get_property = gv_list_get_property;
	object_class->set_property = gv_list_set_property;

	properties[PROP_ITEM_TYPE] =
	        g_param_spec_gtype("item-type", "Item Type",
	                           "The type of the items stored in the list",
	                           G_TYPE_OBJECT,
	                           GV_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE |
	                           G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_LENGTH] =
	        g_param_spec_uint("length", "Length",
	                          "The length of the list",
	                          0, UINT_MAX, 0,
	                          GV_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	g_object_class_install_properties(object_class, PROP_N, properties);

	/* Signals */
	signals[SIGNAL_ITEM_ADDED] =
	        g_signal_new("item-added", G_TYPE_FROM_CLASS(class),
	                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                     0, NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE,
	                     1, G_TYPE_OBJECT);

	signals[SIGNAL_ITEM_REMOVED] =
	        g_signal_new("item-removed", G_TYPE_FROM_CLASS(class),
	                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                     0, NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE,
	                     1, G_TYPE_OBJECT);
}
