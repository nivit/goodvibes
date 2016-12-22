/*
 * Libcaphe
 *
 * Copyright (C) 2016 Arnaud Rebillout
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

#ifndef __LIBCAPHE_CAPHE_DBUS_INHIBITOR_H__
#define __LIBCAPHE_CAPHE_DBUS_INHIBITOR_H__

#include <glib-object.h>

#include "caphe-inhibitor.h"

/* GObject declarations */

#define CAPHE_TYPE_DBUS_INHIBITOR caphe_dbus_inhibitor_get_type()

G_DECLARE_FINAL_TYPE(CapheDbusInhibitor, caphe_dbus_inhibitor, CAPHE, DBUS_INHIBITOR, GObject)

/* Property accessors */

CapheInhibitor *caphe_dbus_inhibitor_new(const gchar *service_id);

#endif /* __LIBCAPHE_CAPHE_DBUS_INHIBITOR_H__ */
