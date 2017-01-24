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

#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include <glib-object.h>

#include "framework/log.h"
#include "framework/gv-framework.h"

GList *gv_framework_errorable_list;

void
gv_framework_cleanup(void)
{
	/* Lists should be empty by now */
	if (gv_framework_errorable_list)
		WARNING("Errorable list not empty, memory is leaked !");
}

void
gv_framework_init(void)
{
	/* Init lists - already intialized to NULL */
}
