/*
 * Libgszn
 *
 * Copyright (C) 2015-2016 Arnaud Rebillout
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

#ifndef __LIBGSZN_GSZN_H__
#define __LIBGSZN_GSZN_H__

#include "gszn-backend.h"
#include "gszn-backend-xml.h"
#include "gszn-backend-keyfile.h"
#include "gszn-param-specs.h"
#include "gszn-serializer.h"
#include "gszn-deserializer.h"

/* Functions */

void gszn_init(void);
void gszn_cleanup(void);

#endif /* __LIBGSZN_GSZN_H__ */
