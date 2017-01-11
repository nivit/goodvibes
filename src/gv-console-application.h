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

#ifndef __GOODVIBES_GV_CONSOLE_APPLICATION_H__
#define __GOODVIBES_GV_CONSOLE_APPLICATION_H__

#include <glib-object.h>
#include <gio/gio.h>

/* GObject declarations */

#define GV_TYPE_CONSOLE_APPLICATION gv_console_application_get_type()

G_DECLARE_FINAL_TYPE(GvConsoleApplication, gv_console_application,
                     GV, CONSOLE_APPLICATION, GApplication)

/* Methods */

GApplication *gv_console_application_new(void);

#endif /* __GOODVIBES_GV_CONSOLE_APPLICATION_H__ */
