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

#ifndef __GOODVIBES_UI_GV_UI_H__
#define __GOODVIBES_UI_GV_UI_H__

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "ui/gv-status-icon.h"

/* Global variables */

extern GSettings    *gv_ui_settings;

extern GvStatusIcon *gv_ui_status_icon;
extern GtkWidget    *gv_ui_main_window;
extern GtkWidget    *gv_ui_prefs_window;

/* Functions */

void gv_ui_init   (GApplication *app, gboolean status_icon_mode);
void gv_ui_cleanup(void);

void gv_ui_present_main       (void);
void gv_ui_present_preferences(void);
void gv_ui_present_about      (void);
void gv_ui_hide               (void);

/* Underlying toolkit */

GOptionGroup *gv_ui_toolkit_init_get_option_group (void);
const gchar  *gv_ui_toolkit_runtime_version_string(void);
const gchar  *gv_ui_toolkit_compile_version_string(void);

#endif /* __GOODVIBES_UI_GV_UI_H__ */
