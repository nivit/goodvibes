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

#ifndef __LIBCAPHE_CAPHE_DBUS_PROXY_H__
#define __LIBCAPHE_CAPHE_DBUS_PROXY_H__

#include <glib-object.h>
#include <gio/gio.h>

/* GObject declarations */

#define CAPHE_TYPE_DBUS_PROXY caphe_dbus_proxy_get_type()

G_DECLARE_FINAL_TYPE(CapheDbusProxy, caphe_dbus_proxy, CAPHE, DBUS_PROXY, GObject)

/* Public methods */

CapheDbusProxy *caphe_dbus_proxy_new(GBusType bus_type, const gchar *well_known_name,
                                     const gchar *object_path, const gchar *interface_name);

/* Property accessors */

GDBusProxy *caphe_dbus_proxy_get_proxy(CapheDbusProxy *self);

#endif /* __LIBCAPHE_CAPHE_DBUS_PROXY_H__ */
