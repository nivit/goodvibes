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

#ifndef __GOODVIBES_FEAT_GV_INHIBITOR_H__
#define __GOODVIBES_FEAT_GV_INHIBITOR_H__

#include <glib-object.h>

#include "framework/gv-feature.h"

/* GObject declarations */

#define GV_TYPE_INHIBITOR gv_inhibitor_get_type()

G_DECLARE_FINAL_TYPE(GvInhibitor, gv_inhibitor, GV, INHIBITOR, GvFeature)

#endif /* __GOODVIBES_FEAT_GV_INHIBITOR_H__ */
