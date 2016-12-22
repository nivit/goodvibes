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

#ifndef __LIBCAPHE_CAPHE_SESSION_DBUS_INVOKATOR_H__
#define __LIBCAPHE_CAPHE_SESSION_DBUS_INVOKATOR_H__

#include <glib-object.h>

#include "caphe-dbus-invokator.h"

/* GObject declarations */

#define CAPHE_TYPE_SESSION_DBUS_INVOKATOR caphe_session_dbus_invokator_get_type()

G_DECLARE_FINAL_TYPE(CapheSessionDbusInvokator, caphe_session_dbus_invokator,
                     CAPHE, SESSION_DBUS_INVOKATOR, CapheDbusInvokator)

/* Methods */

CapheDbusInvokator *caphe_session_dbus_invokator_new(void);

#endif /* __LIBCAPHE_CAPHE_SESSION_DBUS_INVOKATOR_H__ */
