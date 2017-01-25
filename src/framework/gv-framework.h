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

#ifndef __GOODVIBES_FRAMEWORK_GV_FRAMEWORK_H__
#define __GOODVIBES_FRAMEWORK_GV_FRAMEWORK_H__

#include <glib.h>
#include <glib/gi18n.h> /* _() is defined here */

#include "framework/gv-errorable.h"
#include "framework/gv-feature.h"
#include "framework/gv-file-helpers.h"
#include "framework/gv-framework-enum-types.h"
#include "framework/gv-param-specs.h"

#include "framework/log.h"
#include "framework/uri-schemes.h"

/*
 * Global object list
 *
 * This list contains all the global objects. It is completely filled
 * after initialization has been completed, and therefore should only
 * be accessed after this point. In other words, it's probably an error
 * to access this list in a 'constructed()' method, it's too early.
 *
 * Of course this list is 'read-only' and shouldn't be modified.
 */

extern GList *gv_framework_object_list;

/* Functions */

void gv_framework_init    (void);
void gv_framework_cleanup (void);

void gv_framework_register(gpointer object);

#endif /* __GOODVIBES_FRAMEWORK_GV_FRAMEWORK_H__ */
