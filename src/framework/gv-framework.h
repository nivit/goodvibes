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

void gv_framework_init     (void);
void gv_framework_cleanup  (void);

/*
 * List accessors
 *
 * These lists are intended to solve the problem of shared resource
 * between core and ui.
 * Both core and ui provide some objects that can be grouped in categories:
 * errorables.
 * The code that care about these objects needs to access all of them easily,
 * and doesn't care whether the object belongs to the core or the ui.
 * To solve that, the framework provides global lists, that are filled
 * at init time by the core and the ui.
 * These list are supposed to be constant after init time: they shouldn't
 * be modified, no object should be added or removed.
 */

extern GList *gv_framework_errorable_list;

#define gv_framework_errorables_append(item)     \
	gv_framework_errorable_list = g_list_append(gv_framework_errorable_list, item)

#define gv_framework_errorables_remove(item)     \
	gv_framework_errorable_list = g_list_remove(gv_framework_errorable_list, item)

#endif /* __GOODVIBES_FRAMEWORK_GV_FRAMEWORK_H__ */
