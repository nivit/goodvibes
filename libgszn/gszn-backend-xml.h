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

#ifndef __LIBGSZN_GSZN_BACKEND_XML_H__
#define __LIBGSZN_GSZN_BACKEND_XML_H__

#include <glib-object.h>

#include "gszn-backend.h"

/* GObject declarations */

#define GSZN_TYPE_BACKEND_XML gszn_backend_xml_get_type()

G_DECLARE_FINAL_TYPE(GsznBackendXml, gszn_backend_xml, GSZN, BACKEND_XML, GsznBackend)

/* Global methods */

void gszn_backend_xml_cleanup(void);

#endif /* __LIBGSZN_GSZN_BACKEND_XML_H__ */
