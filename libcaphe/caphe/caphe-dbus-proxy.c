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

#include <stdlib.h>
#include <string.h>

#include <glib-object.h>
#include <gio/gio.h>

#include "caphe-trace.h"
#include "caphe-dbus-proxy.h"

#define debug(self, fmt, ...)   \
	g_debug("[%s]: " fmt, strrchr(self->priv->well_known_name, '.') + 1, ##__VA_ARGS__)

#define warning(self, fmt, ...) \
	g_warning("[%s]: " fmt, strrchr(self->priv->well_known_name, '.') + 1, ##__VA_ARGS__)

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Construct-only properties */
	PROP_BUS_TYPE,
	PROP_WELL_KNOWN_NAME,
	PROP_OBJECT_PATH,
	PROP_INTERFACE_NAME,
	/* Readable properties */
	PROP_PROXY,
	/* Total number of properties */
	LAST_PROP
};

static GParamSpec *properties[LAST_PROP];

/*
 * GObject definitions
 */

struct _CapheDbusProxyPrivate {
	/* Properties */
	GBusType    bus_type;
	gchar      *well_known_name;
	gchar      *object_path;
	gchar      *interface_name;
	/* Internal */
	guint       watch_id;
	GDBusProxy *proxy;
};

typedef struct _CapheDbusProxyPrivate CapheDbusProxyPrivate;

struct _CapheDbusProxy {
	/* Parent instance structure */
	GObject                parent_instance;
	/* Private data */
	CapheDbusProxyPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(CapheDbusProxy, caphe_dbus_proxy, G_TYPE_OBJECT)

/*
 * Property setters
 */

static void
caphe_dbus_proxy_set_proxy(CapheDbusProxy *self, GDBusProxy *proxy)
{
	CapheDbusProxyPrivate *priv = self->priv;

	g_set_object(&priv->proxy, proxy);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_PROXY]);
}

/*
 * Callbacks
 */

static void
on_proxy_created(GDBusConnection *connection G_GNUC_UNUSED,
                 GAsyncResult *result,
                 CapheDbusProxy *self)
{
	GDBusProxy *proxy;
	GError *error = NULL;

	debug(self, "Proxy created");

	proxy = g_dbus_proxy_new_finish(result, &error);
	if (error) {
		warning(self, "%s", error->message);
		g_error_free(error);
	}

	caphe_dbus_proxy_set_proxy(self, proxy);
}

static void
on_name_vanished(GDBusConnection *connection G_GNUC_UNUSED,
                 const gchar *name,
                 CapheDbusProxy *self)
{
	debug(self, "Name '%s' vanished", name);

	caphe_dbus_proxy_set_proxy(self, NULL);
}

static void
on_name_appeared(GDBusConnection *connection,
                 const gchar *name,
                 const gchar *name_owner,
                 CapheDbusProxy *self)
{
	CapheDbusProxyPrivate *priv = self->priv;

	debug(self, "Name '%s' appeared (owner %s)", name, name_owner);

	g_dbus_proxy_new(connection,
	                 G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
	                 G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS |
	                 G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
	                 NULL,
	                 priv->well_known_name,
	                 priv->object_path,
	                 priv->interface_name,
	                 NULL,
	                 (GAsyncReadyCallback) on_proxy_created,
	                 self);
}

/*
 * Property accessors
 */

GDBusProxy *
caphe_dbus_proxy_get_proxy(CapheDbusProxy *self)
{
	CapheDbusProxyPrivate *priv = self->priv;

	return priv->proxy;
}

static void
caphe_dbus_proxy_set_bus_type(CapheDbusProxy *self, GBusType bus_type)
{
	CapheDbusProxyPrivate *priv = self->priv;

	/* Construct-only property */
	g_assert(priv->bus_type == G_BUS_TYPE_NONE);
	g_assert(bus_type != G_BUS_TYPE_NONE);

	priv->bus_type = bus_type;
}

static void
caphe_dbus_proxy_set_well_known_name(CapheDbusProxy *self, const gchar *name)
{
	CapheDbusProxyPrivate *priv = self->priv;

	/* Construct-only property */
	g_assert(priv->well_known_name == NULL);
	g_assert(name != NULL);

	priv->well_known_name = g_strdup(name);
}

static void
caphe_dbus_proxy_set_object_path(CapheDbusProxy *self, const gchar *object_path)
{
	CapheDbusProxyPrivate *priv = self->priv;

	/* Construct-only property */
	g_assert(priv->object_path == NULL);
	g_assert(object_path != NULL);

	priv->object_path = g_strdup(object_path);
}

static void
caphe_dbus_proxy_set_interface_name(CapheDbusProxy *self, const gchar *interface_name)
{
	CapheDbusProxyPrivate *priv = self->priv;

	/* Construct-only property */
	g_assert(priv->interface_name == NULL);
	g_assert(interface_name != NULL);

	priv->interface_name = g_strdup(interface_name);
}

static void
caphe_dbus_proxy_get_property(GObject    *object,
                              guint       property_id,
                              GValue     *value G_GNUC_UNUSED,
                              GParamSpec *pspec)
{
	CapheDbusProxy *self = CAPHE_DBUS_PROXY(object);

	switch (property_id) {
	case PROP_PROXY:
		g_value_set_object(value, caphe_dbus_proxy_get_proxy(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
caphe_dbus_proxy_set_property(GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
	CapheDbusProxy *self = CAPHE_DBUS_PROXY(object);

	switch (property_id) {
	case PROP_BUS_TYPE:
		caphe_dbus_proxy_set_bus_type(self, g_value_get_enum(value));
		break;
	case PROP_WELL_KNOWN_NAME:
		caphe_dbus_proxy_set_well_known_name(self, g_value_get_string(value));
		break;
	case PROP_OBJECT_PATH:
		caphe_dbus_proxy_set_object_path(self, g_value_get_string(value));
		break;
	case PROP_INTERFACE_NAME:
		caphe_dbus_proxy_set_interface_name(self, g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Public methods
 */

CapheDbusProxy *
caphe_dbus_proxy_new(GBusType bus_type, const gchar *well_known_name,
                     const gchar *object_path, const gchar *interface_name)
{
	return g_object_new(CAPHE_TYPE_DBUS_PROXY,
	                    "bus-type", bus_type,
	                    "well-known-name", well_known_name,
	                    "object-path", object_path,
	                    "interface-name", interface_name,
	                    NULL);
}

/*
 * GObject methods
 */

static void
caphe_dbus_proxy_finalize(GObject *object)
{
	CapheDbusProxy *self = CAPHE_DBUS_PROXY(object);
	CapheDbusProxyPrivate *priv = self->priv;

	TRACE("%p", object);

	if (priv->watch_id)
		g_bus_unwatch_name(priv->watch_id);

	if (priv->proxy)
		g_object_unref(priv->proxy);

	g_free(priv->well_known_name);
	g_free(priv->object_path);
	g_free(priv->interface_name);

	/* Chain up */
	if (G_OBJECT_CLASS(caphe_dbus_proxy_parent_class)->finalize)
		G_OBJECT_CLASS(caphe_dbus_proxy_parent_class)->finalize(object);
}

static void
caphe_dbus_proxy_constructed(GObject *object)
{
	CapheDbusProxy *self = CAPHE_DBUS_PROXY(object);
	CapheDbusProxyPrivate *priv = self->priv;

	TRACE("%p", object);

	debug(self, "Starting to watch '%s'", priv->well_known_name);

	priv->watch_id = g_bus_watch_name(priv->bus_type,
	                                  priv->well_known_name,
	                                  G_BUS_NAME_WATCHER_FLAGS_NONE,
	                                  (GBusNameAppearedCallback) on_name_appeared,
	                                  (GBusNameVanishedCallback) on_name_vanished,
	                                  self,
	                                  NULL);

	/* Chain up */
	if (G_OBJECT_CLASS(caphe_dbus_proxy_parent_class)->constructed)
		G_OBJECT_CLASS(caphe_dbus_proxy_parent_class)->constructed(object);
}

static void
caphe_dbus_proxy_init(CapheDbusProxy *self)
{
	CapheDbusProxyPrivate *priv = caphe_dbus_proxy_get_instance_private(self);

	TRACE("%p", self);

	/* Construct-only properties, initialized later on */
	priv->bus_type = G_BUS_TYPE_NONE;
	priv->well_known_name = NULL;
	priv->object_path = NULL;
	priv->interface_name = NULL;

	/* Save private pointer */
	self->priv = priv;
}

static void
caphe_dbus_proxy_class_init(CapheDbusProxyClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = caphe_dbus_proxy_finalize;
	object_class->constructed = caphe_dbus_proxy_constructed;

	/* Install properties */
	object_class->get_property = caphe_dbus_proxy_get_property;
	object_class->set_property = caphe_dbus_proxy_set_property;

	properties[PROP_BUS_TYPE] =
	        g_param_spec_enum("bus-type", "Bus Type", NULL,
	                          G_TYPE_BUS_TYPE, G_BUS_TYPE_NONE,
	                          G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE |
	                          G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_WELL_KNOWN_NAME] =
	        g_param_spec_string("well-known-name", "Well-Known Bus Name", NULL,
	                            NULL,
	                            G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE |
	                            G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_OBJECT_PATH] =
	        g_param_spec_string("object-path", "Object Path", NULL,
	                            NULL,
	                            G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE |
	                            G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_INTERFACE_NAME] =
	        g_param_spec_string("interface-name", "Interface Name", NULL,
	                            NULL,
	                            G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE |
	                            G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_PROXY] =
	        g_param_spec_object("proxy", "Proxy", NULL,
	                            G_TYPE_DBUS_PROXY,
	                            G_PARAM_STATIC_STRINGS | G_PARAM_READABLE);

	g_object_class_install_properties(object_class, LAST_PROP, properties);
}
