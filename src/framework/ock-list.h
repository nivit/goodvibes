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

#ifndef __OVERCOOKED_FRAMEWORK_OCK_LIST_H__
#define __OVERCOOKED_FRAMEWORK_OCK_LIST_H__

#include <glib-object.h>

/* GObject declarations */

#define OCK_TYPE_LIST ock_list_get_type()

G_DECLARE_DERIVABLE_TYPE(OckList, ock_list, OCK, LIST, GObject)

/* Data types */

struct _OckListClass {
	/* Parent class */
	GObjectClass parent_class;
};

/* Methods */

OckList  *ock_list_new  (GType item_type);

GList    *ock_list_peek (OckList *self);
gchar    *ock_list_print(OckList *self);

void      ock_list_append_object(OckList *self, GObject *object);
void      ock_list_remove_object(OckList *self, GObject *object);

#define ock_list_append(self, item) ock_list_append_object(self, G_OBJECT(item))
#define ock_list_remove(self, item) ock_list_remove_object(self, G_OBJECT(item))

gboolean  ock_list_is_empty  (OckList *self);

GObject *ock_list_find       (OckList *self, GObject *item);
GObject *ock_list_find_by_type_name(OckList *self, const gchar *type_name);
GObject *ock_list_find_by_property_string(OckList *self, const gchar *property_name,
                                          const gchar *value_to_match);



void ock_list_foreach_connect           (OckList *self, const gchar *signal_name,
                                         GCallback callback, gpointer data);
void ock_list_foreach_disconnect_by_data(OckList *self, gpointer data);

/* Property accessors */

GType ock_list_get_item_type (OckList *self);
guint ock_list_get_length    (OckList *self);

/* Iterator methods */

typedef struct _OckListIter OckListIter;

OckListIter *ock_list_iter_new (OckList *self);
void         ock_list_iter_free(OckListIter *iter);
gboolean     ock_list_iter_loop(OckListIter *iter, GObject **item);

#endif /* __OVERCOOKED_FRAMEWORK_OCK_LIST_H__ */
