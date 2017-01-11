/*
 * Goodvibes Radio Player
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

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "additions/glib-object.h"

#include "framework/log.h"
#include "framework/gv-feature.h"
#include "framework/gv-param-specs.h"

#include "feat/gv-dbus-server.h"

#define MAX_INTERFACES 4

#undef DEBUG_INTERFACES

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Properties - refer to class_init() for more details */
	PROP_DBUS_NAME,
	PROP_DBUS_PATH,
	PROP_DBUS_INTROSPECTION,
	PROP_DBUS_INTERFACE_TABLE,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * GObject definitions
 */

struct _GvDbusServerPrivate {
	/* Properties */
	const gchar      *name;
	const gchar      *path;
	const gchar      *introspection;
	GvDbusInterface *interface_table;
	/* Dbus stuff */
	GDBusNodeInfo    *introspection_data;
	guint             bus_owner_id;
	GDBusConnection  *bus_connection;
	guint             registration_ids[MAX_INTERFACES + 1];
};

typedef struct _GvDbusServerPrivate GvDbusServerPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(GvDbusServer, gv_dbus_server, GV_TYPE_FEATURE)

/*
 * Debug helpers
 */

#ifdef DEBUG_INTERFACES
static void
debug_interfaces(GvDbusServer *self)
{
	GvDbusServerPrivate    *priv = gv_dbus_server_get_instance_private(self);
	GDBusNodeInfo           *info = priv->introspection_data;
	const GvDbusInterface  *iface;
	const GvDbusMethod     *method;
	const GvDbusProperty   *prop;
	GDBusInterfaceInfo     **g_ifaces;
	GDBusInterfaceInfo      *g_iface;
	GDBusMethodInfo        **g_methods;
	GDBusMethodInfo         *g_method;
	GDBusPropertyInfo      **g_props;
	GDBusPropertyInfo       *g_prop;

	/* Find missing things in introspection.
	 * We iterate on methods and properties provided by the implementation,
	 * and ensure they exist in introspection.
	 */

	DEBUG("Debugging introspection data...");

	for (iface = priv->interface_table; iface->name; iface++) {
		g_iface = NULL;

		/* Look for matching interface in introspection data */
		g_ifaces = info->interfaces;
		while (g_ifaces && (g_iface = *g_ifaces++)) {
			if (!g_strcmp0(iface->name, g_iface->name))
				break;
		}

		if (g_iface == NULL)
			ERROR("Implemented interface '%s': no match", iface->name);

		/* Look for matching methods in interface */
		for (method = iface->methods; method && method->name; method++) {
			g_methods = g_iface->methods;
			while (g_methods && (g_method = *g_methods++)) {
				if (!g_strcmp0(method->name, g_method->name))
					break;
			}

			if (g_method == NULL)
				ERROR("Implemented interface '%s': method '%s': no match",
				      iface->name, method->name);
		}

		/* Look for matching properties in interface */
		for (prop = iface->properties; prop && prop->name; prop++) {
			g_prop = NULL;

			g_props = g_iface->properties;
			while (g_props && (g_prop = *g_props++)) {
				if (!g_strcmp0(prop->name, g_prop->name))
					break;
			}

			if (g_prop == NULL)
				ERROR("Implemented interface '%s': property '%s': no match",
				      iface->name, prop->name);

			/* Check that 'get' and 'set' match in the introspection */
			if (prop->get && !(g_prop->flags & G_DBUS_PROPERTY_INFO_FLAGS_READABLE))
				ERROR("Implemented interface '%s': property '%s': get: no match",
				      iface->name, prop->name);

			if (prop->set && !(g_prop->flags & G_DBUS_PROPERTY_INFO_FLAGS_WRITABLE))
				ERROR("Implemented interface '%s': property '%s': set: no match",
				      iface->name, prop->name);
		}
	}

	/* Find missing things in implementation.
	 * We iterate on methods and properties provided by the introspection,
	 * and ensure they are implemented.
	 */

	DEBUG("Debugging implementation...");

	g_ifaces = info->interfaces;
	while (g_ifaces && (g_iface = *g_ifaces++)) {
		/* Look for matching interface in implementation */
		for (iface = priv->interface_table; iface->name; iface++) {
			if (!g_strcmp0(g_iface->name, iface->name))
				break;
		}

		if (iface->name == NULL)
			ERROR("Instrospection interface '%s': no match", g_iface->name);

		/* Look for matching methods in implementation */
		g_methods = g_iface->methods;
		while (g_methods && (g_method = *g_methods++)) {
			for (method = iface->methods; method && method->name; method++) {
				if (!g_strcmp0(g_method->name, method->name))
					break;
			}

			if (method == NULL || method->name == NULL)
				ERROR("Introspection interface '%s': method '%s': no match",
				      g_iface->name, g_method->name);
		}

		/* Look for matching properties in implementation */
		g_props = g_iface->properties;
		while (g_props && (g_prop = *g_props++)) {
			for (prop = iface->properties; prop && prop->name; prop++) {
				if (!g_strcmp0(g_prop->name, prop->name))
					break;
			}

			if (prop == NULL || prop->name == NULL)
				ERROR("Introspection interface '%s': property '%s': no match",
				      g_iface->name, g_prop->name);

			/* Check that 'get' and 'set' match in the implementation */
			if ((g_prop->flags & G_DBUS_PROPERTY_INFO_FLAGS_READABLE) && !prop->get)
				ERROR("Introspection interface '%s': property '%s': "
				      "get is not implemented", g_iface->name, g_prop->name);

			if ((g_prop->flags & G_DBUS_PROPERTY_INFO_FLAGS_WRITABLE) && !prop->set)
				ERROR("Introspection interface '%s': property '%s': "
				      "set is not implemented", g_iface->name, g_prop->name);
		}
	}
}
#endif

/*
 * GDBus helpers
 */

static void
handle_method_call(GDBusConnection       *connection,
                   const gchar           *sender,
                   const gchar           *object_path,
                   const gchar           *interface_name,
                   const gchar           *method_name,
                   GVariant              *parameters,
                   GDBusMethodInvocation *invocation,
                   gpointer               user_data)
{
	GvDbusServer          *self = GV_DBUS_SERVER(user_data);
	GvDbusServerPrivate   *priv = gv_dbus_server_get_instance_private(self);
	const GvDbusInterface *iface;
	const GvDbusMethod    *method;
	GVariant               *ret = NULL;
	GError                 *error = NULL;
	const gchar            *bus_name = connection ?
	                                   g_dbus_connection_get_unique_name(connection) :
	                                   "(null)";

	TRACE("%s, %s, %s, %s, %s, ...",
	      bus_name, sender, object_path, interface_name, method_name);

	/* Iterate over interfaces */
	for (iface = priv->interface_table; iface && iface->name; iface++) {
		if (g_strcmp0(iface->name, interface_name))
			continue;

		/* Iterate over methods */
		for (method = iface->methods; method && method->name; method++) {
			if (g_strcmp0(method->name, method_name))
				continue;

			if (method->call)
				ret = method->call(self, parameters, &error);
			else
				g_set_error(&error, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
				            "Method is not implemented.");

			break;
		}

		/* Check if method was found */
		if (method == NULL || method->name == NULL)
			g_set_error(&error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
			            "Method not found.");

		break;
	}

	/* Check if interface was found */
	if (iface == NULL || iface->name == NULL)
		g_set_error(&error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_INTERFACE,
		            "Interface not found.");

	/* Return with error if any */
	if (error) {
		g_dbus_method_invocation_return_gerror(invocation, error);
		g_error_free(error);
		return;
	}

	/* Return value if any */
	if (ret == NULL)
		g_dbus_method_invocation_return_value(invocation, NULL);
	else
		g_dbus_method_invocation_return_value(invocation,
		                                      g_variant_new_tuple(&ret, 1));
}

static GVariant *
handle_get_property(GDBusConnection  *connection,
                    const gchar      *sender,
                    const gchar      *object_path,
                    const gchar      *interface_name,
                    const gchar      *property_name,
                    GError          **error,
                    gpointer          user_data)
{
	GvDbusServer          *self = GV_DBUS_SERVER(user_data);
	GvDbusServerPrivate   *priv = gv_dbus_server_get_instance_private(self);
	const GvDbusInterface *iface;
	const GvDbusProperty  *prop;
	const gchar            *bus_name = connection ?
	                                   g_dbus_connection_get_unique_name(connection) : "(null)";

	TRACE("%s, %s, %s, %s, %s, ...",
	      bus_name, sender, object_path, interface_name, property_name);

	/* Iterate over interfaces */
	for (iface = priv->interface_table; iface && iface->name; iface++) {
		if (g_strcmp0(iface->name, interface_name))
			continue;

		/* Iterate over properties */
		for (prop = iface->properties; prop && prop->name; prop++) {
			if (g_strcmp0(prop->name, property_name))
				continue;

			if (prop->get == NULL) {
				g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
				            "Property reader is not implemented.");
				return NULL;
			}

			return prop->get(self);
		}

		/* Property not found */
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY,
		            "Property not found.");

		return NULL;
	}

	/* Interface not found */
	g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_INTERFACE,
	            "Interface not found.");

	return NULL;
}

static gboolean
handle_set_property(GDBusConnection  *connection,
                    const gchar      *sender,
                    const gchar      *object_path,
                    const gchar      *interface_name,
                    const gchar      *property_name,
                    GVariant         *value,
                    GError          **error,
                    gpointer          user_data)
{
	GvDbusServer          *self = GV_DBUS_SERVER(user_data);
	GvDbusServerPrivate   *priv = gv_dbus_server_get_instance_private(self);
	const GvDbusInterface *iface = NULL;
	const GvDbusProperty  *prop = NULL;
	const gchar            *bus_name = connection ?
	                                   g_dbus_connection_get_unique_name(connection) : "(null)";

	TRACE("%s, %s, %s, %s, %s, ...",
	      bus_name, sender, object_path, interface_name, property_name);

	/* Iterate over interfaces */
	for (iface = priv->interface_table; iface && iface->name; iface++) {
		if (g_strcmp0(iface->name, interface_name))
			continue;

		/* Iterate over properties */
		for (prop = iface->properties; prop && prop->name; prop++) {
			if (g_strcmp0(prop->name, property_name))
				continue;

			if (prop->set == NULL) {
				g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
				            "Property writer is not implemented.");
				return FALSE;
			}

			return prop->set(self, value, error);
		}

		/* Property not found */
		g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY,
		            "Property not found.");

		return FALSE;
	}

	/* Interface not found */
	g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_INTERFACE,
	            "Interface not found.");

	return FALSE;
}

static const
GDBusInterfaceVTable interface_vtable = {
	.method_call  = handle_method_call,
	.get_property = handle_get_property,
	.set_property = handle_set_property,
};

/*
 * Private methods
 */

static void
gv_dbus_server_register_objects(GvDbusServer *self)
{
	GvDbusServerPrivate *priv = gv_dbus_server_get_instance_private(self);
	GDBusInterfaceInfo **interfaces = priv->introspection_data->interfaces;
	GDBusInterfaceInfo *interface;
	guint i;

	// TODO: checks

	i = 0;
	while (interfaces && (interface = *interfaces++)) {
		guint id;

		g_assert(i < MAX_INTERFACES);

		id = g_dbus_connection_register_object(priv->bus_connection,
		                                       priv->path,
		                                       interface,
		                                       &interface_vtable,
		                                       self,
		                                       NULL,
		                                       NULL);
		g_assert(id > 0);

		priv->registration_ids[i++] = id;

		INFO("Interface '%s' registered", interface->name);
	}
}

static void
gv_dbus_server_unregister_objects(GvDbusServer *self)
{
	GvDbusServerPrivate *priv = gv_dbus_server_get_instance_private(self);
	guint i;

	for (i = 0; priv->registration_ids[i] > 0; i++) {
		g_dbus_connection_unregister_object(priv->bus_connection,
		                                    priv->registration_ids[i]);
		priv->registration_ids[i] = 0;
	}
}

/*
 * GDBus signal handlers
 */

static void
on_bus_acquired(GDBusConnection *connection,
                const gchar     *name,
                gpointer         user_data)
{
	GvDbusServer         *self = GV_DBUS_SERVER(user_data);
	GvDbusServerPrivate  *priv = gv_dbus_server_get_instance_private(self);
	const gchar           *bus_name = connection ?
	                                  g_dbus_connection_get_unique_name(connection) :
	                                  "(null)";

	TRACE("%s, %s, %p", bus_name, name, user_data);

	/* Save DBus connection */
	priv->bus_connection = g_object_ref(connection);

	/* Register objects */
	gv_dbus_server_register_objects(self);
}

static void
on_name_acquired(GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data G_GNUC_UNUSED)
{
	const gchar *bus_name = connection ?
	                        g_dbus_connection_get_unique_name(connection) :
	                        "(null)";

	TRACE("%s, %s, ...", bus_name, name);
}

static void
on_name_lost(GDBusConnection *connection,
             const gchar     *name,
             gpointer         user_data G_GNUC_UNUSED)
{
	const gchar *bus_name = connection ?
	                        g_dbus_connection_get_unique_name(connection) :
	                        "(null)";

	TRACE("%s, %s, ...", bus_name, name);

	if (connection == NULL)
		WARNING("Name '%s' can't connect to the bus", name);
	else
		WARNING("Name '%s' lost on the bus", name);
}

/*
 * Property accessors
 */

void
gv_dbus_server_set_dbus_name(GvDbusServer *self, const gchar *value)
{
	GvDbusServerPrivate *priv = gv_dbus_server_get_instance_private(self);

	g_assert(priv->name == NULL);
	priv->name = value;
}

void
gv_dbus_server_set_dbus_path(GvDbusServer *self, const gchar *value)
{
	GvDbusServerPrivate *priv = gv_dbus_server_get_instance_private(self);

	g_assert(priv->path == NULL);
	priv->path = value;
}

void
gv_dbus_server_set_dbus_introspection(GvDbusServer *self, const gchar *value)
{
	GvDbusServerPrivate *priv = gv_dbus_server_get_instance_private(self);

	g_assert(priv->introspection == NULL);
	priv->introspection = value;
}

void
gv_dbus_server_set_dbus_interface_table(GvDbusServer *self, GvDbusInterface *value)
{
	GvDbusServerPrivate *priv = gv_dbus_server_get_instance_private(self);

	g_assert(priv->interface_table == NULL);
	priv->interface_table = value;
}

static void
gv_dbus_server_get_property(GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
}

static void
gv_dbus_server_set_property(GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
	GvDbusServer *self = GV_DBUS_SERVER(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_DBUS_NAME:
		gv_dbus_server_set_dbus_name(self, g_value_get_string(value));
		break;
	case PROP_DBUS_PATH:
		gv_dbus_server_set_dbus_path(self, g_value_get_string(value));
		break;
	case PROP_DBUS_INTROSPECTION:
		gv_dbus_server_set_dbus_introspection(self, g_value_get_string(value));
		break;
	case PROP_DBUS_INTERFACE_TABLE:
		gv_dbus_server_set_dbus_interface_table(self, g_value_get_pointer(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Public methods
 */

void
gv_dbus_server_emit_signal(GvDbusServer *self, const gchar *interface_name,
                           const gchar *signal_name, GVariant *parameters)
{
	GvDbusServerPrivate *priv = gv_dbus_server_get_instance_private(self);
	GError *error = NULL;

	/* We're not sure to have a connection to dbus. Connection might fail
	 * (for example, if the name is already owned). Or, early at startup,
	 * we might still be waiting for the connection to finish when we're
	 * asked to send the first signals. In any case, we must check that the
	 * connection exists before using it.
	 */
	if (priv->bus_connection == NULL)
		return;

	g_dbus_connection_emit_signal(priv->bus_connection, NULL, priv->path,
	                              interface_name, signal_name, parameters, &error);
	if (error) {
		WARNING("Failed to emit dbus signal: %s", error->message);
		g_error_free(error);
	}
}

void
gv_dbus_server_emit_signal_property_changed(GvDbusServer *self, const gchar *interface_name,
                const gchar *property_name, GVariant *value)
{
	GVariantBuilder b;

	g_variant_builder_init(&b, G_VARIANT_TYPE("a{sv}"));
	g_variant_builder_add(&b, "{sv}", property_name, value);

	GVariant *tuples[] = {
		g_variant_new_string(interface_name),
		g_variant_builder_end(&b),
		g_variant_new_strv(NULL, 0)
	};

	gv_dbus_server_emit_signal(self, "org.freedesktop.DBus.Properties",
	                           "PropertiesChanged", g_variant_new_tuple(tuples, 3));
}

GvDbusServer *
gv_dbus_server_new(void)
{
	return g_object_new(GV_TYPE_DBUS_SERVER, NULL);
}

/*
 * Feature methods
 */

static void
gv_dbus_server_disable(GvFeature *feature)
{
	GvDbusServer *self = GV_DBUS_SERVER(feature);
	GvDbusServerPrivate *priv = gv_dbus_server_get_instance_private(self);

	/* Unref DBus connection & objects registered */
	if (priv->bus_connection != NULL) {
		gv_dbus_server_unregister_objects(self);

		g_object_unref(priv->bus_connection);
		priv->bus_connection = NULL;
	}

	/* Unown name */
	if (priv->bus_owner_id != 0) {
		g_bus_unown_name(priv->bus_owner_id);
		priv->bus_owner_id = 0;
	}

	/* Chain up */
	GV_FEATURE_CHAINUP_DISABLE(gv_dbus_server, feature);
}

static void
gv_dbus_server_enable(GvFeature *feature)
{
	GvDbusServer *self = GV_DBUS_SERVER(feature);
	GvDbusServerPrivate *priv = gv_dbus_server_get_instance_private(self);

	/* Chain up */
	GV_FEATURE_CHAINUP_ENABLE(gv_dbus_server, feature);

	/* We might want to acquire a name or not */
	if (priv->name) {
		/* Acquire a name on the bus (objects will be registered in the callback) */
		priv->bus_owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
		                                    priv->name,
		                                    G_BUS_NAME_OWNER_FLAGS_NONE,
		                                    on_bus_acquired,
		                                    on_name_acquired,
		                                    on_name_lost,
		                                    self,
		                                    NULL);
		g_assert(priv->bus_owner_id > 0);

	} else {
		GDBusConnection *connection;
		GError *err = NULL;

		/* Get connection to the bus */
		connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &err);
		if (err) {
			CRITICAL("Failed to get bus: %s", err->message);
			g_error_free(err);
			return;
		}

		priv->bus_connection = g_object_ref(connection);

		/* Register objects */
		gv_dbus_server_register_objects(self);
	}
}

/*
 * GObject methods
 */

static void
gv_dbus_server_finalize(GObject *object)
{
	GvDbusServer *self = GV_DBUS_SERVER(object);
	GvDbusServerPrivate *priv = gv_dbus_server_get_instance_private(self);

	TRACE("%p", object);

	/* Unref introspection data */
	if (priv->introspection_data != NULL)
		g_dbus_node_info_unref(priv->introspection_data);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(gv_dbus_server, object);
}

static void
gv_dbus_server_constructed(GObject *object)
{
	GvDbusServer *self = GV_DBUS_SERVER(object);
	GvDbusServerPrivate *priv = gv_dbus_server_get_instance_private(self);

	TRACE("%p", object);

	/* The 'name' property might be left to NULL if we don't want to
	 * own a bus name. Other properties, though, must have been set.
	 */
	g_assert(priv->path);
	g_assert(priv->introspection);
	g_assert(priv->interface_table);

	/* Parse XML introspection data */
	priv->introspection_data = g_dbus_node_info_new_for_xml(priv->introspection, NULL);
	g_assert(priv->introspection_data != NULL);

#ifdef DEBUG_INTERFACES
	/* Ensure that the interface table matches the introspection data.
	 * Be sure to enable this test if you work on this part.
	 */
	debug_interfaces(self);
#endif

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(gv_dbus_server, object);
}

static void
gv_dbus_server_init(GvDbusServer *self)
{
	TRACE("%p", self);
}

static void
gv_dbus_server_class_init(GvDbusServerClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	GvFeatureClass *feature_class = GV_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = gv_dbus_server_finalize;
	object_class->constructed = gv_dbus_server_constructed;

	/* Override GvFeature methods */
	feature_class->enable = gv_dbus_server_enable;
	feature_class->disable = gv_dbus_server_disable;

	/* Properties
	 * All the strings are supposed to be static, we won't duplicate them.
	 * This makes a little optimization in memory usage.
	 * See the set() accessors if don't get it.
	 */
	object_class->get_property = gv_dbus_server_get_property;
	object_class->set_property = gv_dbus_server_set_property;

	properties[PROP_DBUS_NAME] =
	        g_param_spec_string("dbus-name", "Dbus well-known name", NULL, NULL,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_WRITABLE);

	properties[PROP_DBUS_PATH] =
	        g_param_spec_string("dbus-path", "Dbus path", NULL, NULL,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_WRITABLE);

	properties[PROP_DBUS_INTROSPECTION] =
	        g_param_spec_string("dbus-introspection", "Dbus instrospection", NULL, NULL,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_WRITABLE);

	properties[PROP_DBUS_INTERFACE_TABLE] =
	        g_param_spec_pointer("dbus-interface-table", "Dbus interface table", NULL,
	                             GV_PARAM_DEFAULT_FLAGS | G_PARAM_WRITABLE);

	g_object_class_install_properties(object_class, PROP_N, properties);
}
