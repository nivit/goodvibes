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

#ifndef __OVERCOOKED_UI_OCK_TRAY_H__
#define __OVERCOOKED_UI_OCK_TRAY_H__

#include <glib-object.h>

/* GObject declarations */

#define OCK_TYPE_TRAY ock_tray_get_type()

G_DECLARE_FINAL_TYPE(OckTray, ock_tray, OCK, TRAY, GObject)

/* Data types */

typedef enum {
	OCK_TRAY_MIDDLE_CLICK_ACTION_UNDEFINED,
	OCK_TRAY_MIDDLE_CLICK_ACTION_TOGGLE,
	OCK_TRAY_MIDDLE_CLICK_ACTION_MUTE,
} OckTrayMiddleClickAction;

typedef enum {
	OCK_TRAY_SCROLL_ACTION_UNDEFINED,
	OCK_TRAY_SCROLL_ACTION_STATION,
	OCK_TRAY_SCROLL_ACTION_VOLUME,
} OckTrayScrollAction;

/* Methods */

OckTray *ock_tray_new(void);

/* Property accessors */

OckTrayMiddleClickAction ock_tray_get_middle_click_action(OckTray *self);
void                     ock_tray_set_middle_click_action(OckTray *self,
                                                          OckTrayMiddleClickAction action);
OckTrayScrollAction      ock_tray_get_scroll_action      (OckTray *self);
void                     ock_tray_set_scroll_action      (OckTray *self,
                                                          OckTrayScrollAction action);

#endif /* __OVERCOOKED_UI_OCK_TRAY_H__ */
