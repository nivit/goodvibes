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
 * This header contains definitions to be used by feat users
 */

#ifndef __GOODVIBES_FEAT_GV_FEAT_H__
#define __GOODVIBES_FEAT_GV_FEAT_H__

#include "framework/gv-feature.h"

/* Functions */

void       gv_feat_init   (void);
void       gv_feat_cleanup(void);

GvFeature *gv_feat_find   (const gchar *name_to_find);

#endif /* __GOODVIBES_FEAT_GV_FEAT_H__ */
