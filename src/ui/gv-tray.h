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

#ifndef __GOODVIBES_UI_GV_TRAY_H__
#define __GOODVIBES_UI_GV_TRAY_H__

#include <glib-object.h>
#include <gtk/gtk.h>

/* GObject declarations */

#define GV_TYPE_TRAY gv_tray_get_type()

G_DECLARE_FINAL_TYPE(GvTray, gv_tray, GV, TRAY, GObject)

/* Data types */

typedef enum {
	GV_TRAY_MIDDLE_CLICK_ACTION_UNDEFINED,
	GV_TRAY_MIDDLE_CLICK_ACTION_TOGGLE,
	GV_TRAY_MIDDLE_CLICK_ACTION_MUTE,
} GvTrayMiddleClickAction;

typedef enum {
	GV_TRAY_SCROLL_ACTION_UNDEFINED,
	GV_TRAY_SCROLL_ACTION_STATION,
	GV_TRAY_SCROLL_ACTION_VOLUME,
} GvTrayScrollAction;

/* Methods */

GvTray *gv_tray_new(GtkWindow *main_window);

/* Property accessors */

GvTrayMiddleClickAction gv_tray_get_middle_click_action(GvTray *self);
void                    gv_tray_set_middle_click_action(GvTray *self,
                                                        GvTrayMiddleClickAction action);
GvTrayScrollAction      gv_tray_get_scroll_action      (GvTray *self);
void                    gv_tray_set_scroll_action      (GvTray *self,
                                                        GvTrayScrollAction action);

#endif /* __GOODVIBES_UI_GV_TRAY_H__ */
