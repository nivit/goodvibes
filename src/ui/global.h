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

#ifndef __OVERCOOKED_UI_GLOBAL_H__
#define __OVERCOOKED_UI_GLOBAL_H__

#include <gtk/gtk.h>

#include "ui/ock-tray.h"
#include "ui/ock-main-window.h"

/*
 * Visual layout, according to
 * https://developer.gnome.org/hig/stable/visual-layout.html
 */

#define OCK_UI_WINDOW_BORDER 18
#define OCK_UI_GROUP_SPACING 18
#define OCK_UI_ELEM_SPACING  6
#define OCK_UI_LABEL_SPACING 12

/*
 * Global variables
 */

extern OckTray   *ock_ui_tray;
extern GtkWidget *ock_ui_main_window;

#endif /* __OVERCOOKED_UI_GLOBAL_H__ */
