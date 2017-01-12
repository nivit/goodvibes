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

#ifndef __GOODVIBES_CORE_GV_CONF_H__
#define __GOODVIBES_CORE_GV_CONF_H__

#include <glib-object.h>

#include "core/gv-player.h"

/* GObject declarations */

#define GV_TYPE_CONF gv_conf_get_type()

G_DECLARE_FINAL_TYPE(GvConf, gv_conf, GV, CONF, GObject)

/* Methods */

GvConf *gv_conf_new    (void);
void     gv_conf_load   (GvConf *self);
void     gv_conf_save   (GvConf *self);
void     gv_conf_apply  (GvConf *self);
void     gv_conf_watch  (GvConf *self);
void     gv_conf_unwatch(GvConf *self);

#endif /* __GOODVIBES_CORE_GV_CONF_H__ */
