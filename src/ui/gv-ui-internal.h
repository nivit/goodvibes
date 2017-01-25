/*
 * Goodvibes Radio Player
 *
 * Copyright (C) 2015-2017 Arnaud Rebillout
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

/*
 * This header contains definitions to be used internally by ui files
 */

#ifndef __GOODVIBES_UI_GV_UI_INTERNAL_H__
#define __GOODVIBES_UI_GV_UI_INTERNAL_H__

#include <gio/gio.h>

#include "ui/gv-status-icon.h"

/* Global variables */

extern GSettings    *gv_ui_settings;

extern GvStatusIcon *gv_ui_status_icon;
extern GtkWidget    *gv_ui_main_window;
extern GtkWidget    *gv_ui_prefs_window;

/*
 * Visual layout, according to:
 * https://developer.gnome.org/hig/stable/visual-layout.html
 */

#define GV_UI_WINDOW_BORDER 18
#define GV_UI_GROUP_SPACING 18
#define GV_UI_ELEM_SPACING  6
#define GV_UI_LABEL_SPACING 12

#endif /* __GOODVIBES_UI_GV_UI_INTERNAL_H__ */
