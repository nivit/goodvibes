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

#ifndef __GOODVIBES_UI_GV_ABOUT_DIALOG_H__
#define __GOODVIBES_UI_GV_ABOUT_DIALOG_H__

#include <gtk/gtk.h>

void gv_show_about_dialog(GtkWindow *parent, const gchar *audio_backend_string,
                          const gchar *ui_toolkit_string);

#endif /* __GOODVIBES_UI_GV_ABOUT_DIALOG_H__ */
