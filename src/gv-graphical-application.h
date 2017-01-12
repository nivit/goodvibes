/*
 * Goodvibes Radio Player
 *
 * Copyright (C) 2017 Arnaud Rebillout
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

#ifndef __GOODVIBES_GV_GRAPHICAL_APPLICATION_H__
#define __GOODVIBES_GV_GRAPHICAL_APPLICATION_H__

#include <glib-object.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

/* GObject declarations */

#define GV_TYPE_GRAPHICAL_APPLICATION gv_graphical_application_get_type()

G_DECLARE_FINAL_TYPE(GvGraphicalApplication, gv_graphical_application,
                     GV, GRAPHICAL_APPLICATION, GtkApplication)

/* Methods */

GApplication *gv_graphical_application_new(const gchar *application_id);

#endif /* __GOODVIBES_GV_GRAPHICAL_APPLICATION_H__ */
