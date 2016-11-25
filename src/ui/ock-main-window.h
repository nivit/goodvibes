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

#ifndef __OVERCOOKED_UI_OCK_MAIN_WINDOW_H__
#define __OVERCOOKED_UI_OCK_MAIN_WINDOW_H__

#include <glib-object.h>
#include <gtk/gtk.h>

/* GObject declarations */

#define OCK_TYPE_MAIN_WINDOW ock_main_window_get_type()

G_DECLARE_FINAL_TYPE(OckMainWindow, ock_main_window, OCK, MAIN_WINDOW, GtkWindow)

/* Methods */

GtkWidget *ock_main_window_new(void);

#endif /* __OVERCOOKED_UI_OCK_MAIN_WINDOW_H__ */
