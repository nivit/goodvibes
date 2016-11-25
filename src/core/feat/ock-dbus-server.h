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

#ifndef __OVERCOOKED_CORE_FEAT_OCK_DBUS_SERVER_H__
#define __OVERCOOKED_CORE_FEAT_OCK_DBUS_SERVER_H__

#include <glib-object.h>

#include "framework/ock-feature.h"

/* GObject declarations */

#define OCK_TYPE_DBUS_SERVER ock_dbus_server_get_type()

G_DECLARE_DERIVABLE_TYPE(OckDbusServer, ock_dbus_server, OCK, DBUS_SERVER, OckFeature)

/* Data types */

struct _OckDbusServerClass {
	/* Parent class */
	OckFeatureClass parent_class;
};

typedef GVariant *(*OckDbusMethodCall)  (OckDbusServer *, GVariant *, GError **);
typedef GVariant *(*OckDbusPropertyGet) (OckDbusServer *);
typedef gboolean  (*OckDbusPropertySet) (OckDbusServer *, GVariant *, GError **);

struct _OckDbusMethod {
	const gchar             *name;
	const OckDbusMethodCall  call;
};

typedef struct _OckDbusMethod OckDbusMethod;

struct _OckDbusProperty {
	const gchar              *name;
	const OckDbusPropertyGet  get;
	const OckDbusPropertySet  set;
};

typedef struct _OckDbusProperty OckDbusProperty;

struct _OckDbusInterface {
	const gchar           *name;
	const OckDbusMethod   *methods;
	const OckDbusProperty *properties;
};

typedef struct _OckDbusInterface OckDbusInterface;

/* Methods */

OckDbusServer *ock_dbus_server_new(void);

void ock_dbus_server_emit_signal(OckDbusServer *self,
                                 const gchar *interface_name,
                                 const gchar *signal_name,
                                 GVariant *parameters);

void ock_dbus_server_emit_signal_property_changed(OckDbusServer *self,
                                                  const gchar *interface_name,
                                                  const gchar *property_name,
                                                  GVariant *value);

/* Property accessors */

void ock_dbus_server_set_dbus_name           (OckDbusServer *self, const gchar *name);
void ock_dbus_server_set_dbus_path           (OckDbusServer *self, const gchar *path);
void ock_dbus_server_set_dbus_introspection  (OckDbusServer *self, const gchar *introspection);
void ock_dbus_server_set_dbus_interface_table(OckDbusServer *self,
                                              OckDbusInterface *interface_table);

#endif /* __OVERCOOKED_CORE_FEAT_OCK_DBUS_SERVER_H__ */
