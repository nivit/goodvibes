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

#ifndef __LIBCAPHE_CAPHE_DBUS_INVOKATOR_H__
#define __LIBCAPHE_CAPHE_DBUS_INVOKATOR_H__

#include <glib-object.h>
#include <gio/gio.h>

/* GObject declarations */

#define CAPHE_TYPE_DBUS_INVOKATOR caphe_dbus_invokator_get_type()

G_DECLARE_DERIVABLE_TYPE(CapheDbusInvokator, caphe_dbus_invokator, CAPHE, DBUS_INVOKATOR, GObject)

struct _CapheDbusInvokatorClass {
	/* Parent class */
	GObjectClass parent_class;

	/* Virtual Methods */
	void     (*inhibit)     (CapheDbusInvokator *self,
	                         const gchar *application,
	                         const gchar *reason);
	void     (*uninhibit)   (CapheDbusInvokator *self);
	gboolean (*is_inhibited)(CapheDbusInvokator *self);
};

/* Methods */

CapheDbusInvokator *caphe_dbus_invokator_new(GType invokator_type, GDBusProxy *proxy);

void caphe_dbus_invokator_inhibit  (CapheDbusInvokator *self,
                                    const gchar *application,
                                    const gchar *reason);
void caphe_dbus_invokator_uninhibit(CapheDbusInvokator *self);

/* Public property accessors */

GDBusProxy *caphe_dbus_invokator_get_proxy    (CapheDbusInvokator *self);
gboolean    caphe_dbus_invokator_get_inhibited(CapheDbusInvokator *self);

#endif /* __LIBCAPHE_CAPHE_DBUS_INVOKATOR_H__ */
