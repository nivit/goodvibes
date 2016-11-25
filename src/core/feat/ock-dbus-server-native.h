/*
 * Overcooked Radio Player
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

#ifndef __OVERCOOKED_CORE_FEAT_OCK_DBUS_SERVER_NATIVE_H__
#define __OVERCOOKED_CORE_FEAT_OCK_DBUS_SERVER_NATIVE_H__

#include <glib-object.h>

#include "framework/ock-feature.h"

#include "core/feat/ock-dbus-server.h"

/* GObject declarations */

#define OCK_TYPE_DBUS_SERVER_NATIVE ock_dbus_server_native_get_type()

G_DECLARE_FINAL_TYPE(OckDbusServerNative, ock_dbus_server_native, \
                     OCK, DBUS_SERVER_NATIVE, OckDbusServer)

#endif /* __OVERCOOKED_CORE_FEAT_OCK_DBUS_SERVER_NATIVE_H__ */
