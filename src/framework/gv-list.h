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

#ifndef __GOODVIBES_FRAMEWORK_GV_LIST_H__
#define __GOODVIBES_FRAMEWORK_GV_LIST_H__

#include <glib-object.h>

/* GObject declarations */

#define GV_TYPE_LIST gv_list_get_type()

G_DECLARE_DERIVABLE_TYPE(GvList, gv_list, GV, LIST, GObject)

/* Data types */

struct _GvListClass {
	/* Parent class */
	GObjectClass parent_class;
};

/* Methods */

GvList  *gv_list_new  (GType item_type);

GList    *gv_list_peek (GvList *self);
gchar    *gv_list_print(GvList *self);

void      gv_list_append_object(GvList *self, GObject *object);
void      gv_list_remove_object(GvList *self, GObject *object);

#define gv_list_append(self, item) gv_list_append_object(self, G_OBJECT(item))
#define gv_list_remove(self, item) gv_list_remove_object(self, G_OBJECT(item))

gboolean  gv_list_is_empty  (GvList *self);

GObject *gv_list_find       (GvList *self, GObject *item);
GObject *gv_list_find_by_type_name(GvList *self, const gchar *type_name);
GObject *gv_list_find_by_property_string(GvList *self, const gchar *property_name,
                                         const gchar *value_to_match);



void gv_list_foreach_connect           (GvList *self, const gchar *signal_name,
                                        GCallback callback, gpointer data);
void gv_list_foreach_disconnect_by_data(GvList *self, gpointer data);

/* Property accessors */

GType gv_list_get_item_type (GvList *self);
guint gv_list_get_length    (GvList *self);

/* Iterator methods */

typedef struct _GvListIter GvListIter;

GvListIter *gv_list_iter_new (GvList *self);
void         gv_list_iter_free(GvListIter *iter);
gboolean     gv_list_iter_loop(GvListIter *iter, GObject **item);

#endif /* __GOODVIBES_FRAMEWORK_GV_LIST_H__ */
