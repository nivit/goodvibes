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

#include <glib-object.h>
#include <gio/gio.h>

#include "caphe-trace.h"
#include "caphe-login-dbus-invokator.h"
#include "caphe-power-dbus-invokator.h"
#include "caphe-session-dbus-invokator.h"
#include "caphe-dbus-invokator.h"
#include "caphe-dbus-proxy.h"
#include "caphe-dbus-inhibitor.h"

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Construct-only properties */
	PROP_SERVICE_ID,
	/* Number of installable properties */
	LAST_INSTALLABLE_PROP,
	/* Overriden properties */
	PROP_AVAILABLE,
	PROP_INHIBITED,
	/* Total number of properties */
	LAST_PROP
};

static GParamSpec *properties[LAST_PROP];

/*
 * GObject definitions
 */

struct _CapheDbusInhibitorPrivate {
	/* Properties */
	gchar              *service_id;
	/* Internal */
	CapheDbusProxy     *proxy;
	GType               invokator_type;
	CapheDbusInvokator *invokator;
};

typedef struct _CapheDbusInhibitorPrivate CapheDbusInhibitorPrivate;

struct _CapheDbusInhibitor {
	/* Parent instance structure */
	GObject parent_instance;
	/* Private data */
	CapheDbusInhibitorPrivate *priv;
};

static void caphe_inhibitor_interface_init(CapheInhibitorInterface *iface);

G_DEFINE_TYPE_WITH_CODE(CapheDbusInhibitor, caphe_dbus_inhibitor, G_TYPE_OBJECT,
                        G_ADD_PRIVATE(CapheDbusInhibitor)
                        G_IMPLEMENT_INTERFACE(CAPHE_TYPE_INHIBITOR,
                                        caphe_inhibitor_interface_init))

/*
 * Signal handlers
 */

static void
on_caphe_dbus_invokator_notify_inhibited(CapheDbusInvokator *caphe_invokator G_GNUC_UNUSED,
                GParamSpec *pspec G_GNUC_UNUSED,
                CapheDbusInhibitor *self)
{
	/* Notify that inhibited status changed */
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_INHIBITED]);
}

static void
on_caphe_dbus_proxy_notify_proxy(CapheDbusProxy *caphe_proxy,
                                 GParamSpec *pspec G_GNUC_UNUSED,
                                 CapheDbusInhibitor *self)
{
	CapheDbusInhibitorPrivate *priv = self->priv;
	GDBusProxy *dbus_proxy = caphe_dbus_proxy_get_proxy(caphe_proxy);

	/* If the proxy disappeared, we must dispose of the invokator.
	 * Otherwise, we must create an invokator.
	 */
	if (dbus_proxy == NULL) {
		if (priv->invokator) {
			g_object_unref(priv->invokator);
			priv->invokator = NULL;
		}

	} else {
		if (priv->invokator == NULL) {
			priv->invokator = caphe_dbus_invokator_new
			                  (priv->invokator_type, dbus_proxy);
			g_signal_connect(priv->invokator, "notify::inhibited",
			                 (GCallback) on_caphe_dbus_invokator_notify_inhibited,
			                 self);
		}
	}

	/* Notify that availability changed */
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_AVAILABLE]);
}

/*
 * Inhibitor methods
 */

static void
caphe_dbus_inhibitor_inhibit(CapheInhibitor *inhibitor, const gchar *application,
                             const gchar *reason)
{
	CapheDbusInhibitor *self = CAPHE_DBUS_INHIBITOR(inhibitor);
	CapheDbusInhibitorPrivate *priv = self->priv;

	g_return_if_fail(application != NULL);
	g_return_if_fail(reason != NULL);
	g_return_if_fail(priv->invokator != NULL);

	caphe_dbus_invokator_inhibit(priv->invokator, application, reason);
}

static void
caphe_dbus_inhibitor_uninhibit(CapheInhibitor *inhibitor)
{
	CapheDbusInhibitor *self = CAPHE_DBUS_INHIBITOR(inhibitor);
	CapheDbusInhibitorPrivate *priv = self->priv;

	g_return_if_fail(priv->invokator != NULL);

	caphe_dbus_invokator_uninhibit(priv->invokator);
}

/*
 * Inhibitor property accessors
 */

static gboolean
caphe_dbus_inhibitor_get_available(CapheInhibitor *inhibitor)
{
	CapheDbusInhibitor *self = CAPHE_DBUS_INHIBITOR(inhibitor);
	CapheDbusInhibitorPrivate *priv = self->priv;

	return priv->invokator ? TRUE : FALSE;
}

static gboolean
caphe_dbus_inhibitor_get_inhibited(CapheInhibitor *inhibitor)
{
	CapheDbusInhibitor *self = CAPHE_DBUS_INHIBITOR(inhibitor);
	CapheDbusInhibitorPrivate *priv = self->priv;

	if (priv->invokator == NULL)
		return FALSE;

	return caphe_dbus_invokator_get_inhibited(priv->invokator);
}

/*
 * Property accessors
 */

static void
caphe_dbus_inhibitor_set_service_id(CapheDbusInhibitor *self, const gchar *service_id)
{
	CapheDbusInhibitorPrivate *priv = self->priv;

	/* Construct-only property */
	g_assert(priv->service_id == NULL);
	g_assert(service_id != NULL);

	priv->service_id = g_strdup(service_id);
}

static void
caphe_dbus_inhibitor_get_property(GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
	CapheInhibitor *inhibitor = CAPHE_INHIBITOR(object);

	switch (property_id) {
	/* Inhibitor */
	case PROP_AVAILABLE:
		g_value_set_boolean(value, caphe_dbus_inhibitor_get_available(inhibitor));
		break;
	case PROP_INHIBITED:
		g_value_set_boolean(value, caphe_dbus_inhibitor_get_inhibited(inhibitor));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
caphe_dbus_inhibitor_set_property(GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
	CapheDbusInhibitor *self = CAPHE_DBUS_INHIBITOR(object);

	switch (property_id) {
	case PROP_SERVICE_ID:
		caphe_dbus_inhibitor_set_service_id(self, g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Public methods
 */

CapheInhibitor *
caphe_dbus_inhibitor_new(const gchar *service_id)
{
	return g_object_new(CAPHE_TYPE_DBUS_INHIBITOR,
	                    "service-id", service_id,
	                    NULL);
}

/*
 * GObject methods
 */

static void
caphe_dbus_inhibitor_finalize(GObject *object)
{
	CapheDbusInhibitor *self = CAPHE_DBUS_INHIBITOR(object);
	CapheDbusInhibitorPrivate *priv = self->priv;

	TRACE("%p", object);

	if (priv->invokator)
		g_object_unref(priv->invokator);

	if (priv->proxy)
		g_object_unref(priv->proxy);

	g_free(priv->service_id);

	/* Chain up */
	if (G_OBJECT_CLASS(caphe_dbus_inhibitor_parent_class)->finalize)
		G_OBJECT_CLASS(caphe_dbus_inhibitor_parent_class)->finalize(object);
}

static void
caphe_dbus_inhibitor_constructed(GObject *object)
{
	CapheDbusInhibitor *self = CAPHE_DBUS_INHIBITOR(object);
	CapheDbusInhibitorPrivate *priv = self->priv;

	TRACE("%p", object);

	/* Create the dbus proxy */
	if (!g_strcmp0(priv->service_id, "SessionManager")) {
		priv->invokator_type = CAPHE_TYPE_SESSION_DBUS_INVOKATOR;
		priv->proxy = caphe_dbus_proxy_new(G_BUS_TYPE_SESSION,
		                                   "org.gnome.SessionManager",
		                                   "/org/gnome/SessionManager",
		                                   "org.gnome.SessionManager");

	} else if (!g_strcmp0(priv->service_id, "PowerManagement")) {
		priv->invokator_type = CAPHE_TYPE_POWER_DBUS_INVOKATOR;
		priv->proxy = caphe_dbus_proxy_new(G_BUS_TYPE_SESSION,
		                                   "org.freedesktop.PowerManagement",
		                                   "/org/freedesktop/PowerManagement/Inhibit",
		                                   "org.freedesktop.PowerManagement.Inhibit");

	} else if (!g_strcmp0(priv->service_id, "Login1")) {
		priv->invokator_type = CAPHE_TYPE_LOGIN_DBUS_INVOKATOR;
		priv->proxy = caphe_dbus_proxy_new(G_BUS_TYPE_SYSTEM,
		                                   "org.freedesktop.login1",
		                                   "/org/freedesktop/login1",
		                                   "org.freedesktop.login1.Manager");
	} else {
		g_error("Invalid dbus service id '%s'", priv->service_id);
		/* Program execution stops here */

	}

	/* Connect to the 'proxy' signal to be notified as soon as the proxy
	 * is created. The signal is ensured to be sent once at init time.
	 */
	g_signal_connect(priv->proxy, "notify::proxy",
	                 (GCallback) on_caphe_dbus_proxy_notify_proxy, self);

	/* Chain up */
	if (G_OBJECT_CLASS(caphe_dbus_inhibitor_parent_class)->constructed)
		G_OBJECT_CLASS(caphe_dbus_inhibitor_parent_class)->constructed(object);
}

static void
caphe_dbus_inhibitor_init(CapheDbusInhibitor *self)
{
	CapheDbusInhibitorPrivate *priv = caphe_dbus_inhibitor_get_instance_private(self);

	TRACE("%p", self);

	/* Construct-only properties, initialized later on */
	priv->service_id = NULL;

	/* Set private pointer */
	self->priv = priv;
}

static void
caphe_dbus_inhibitor_class_init(CapheDbusInhibitorClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize    = caphe_dbus_inhibitor_finalize;
	object_class->constructed = caphe_dbus_inhibitor_constructed;

	/* Install properties */
	object_class->get_property = caphe_dbus_inhibitor_get_property;
	object_class->set_property = caphe_dbus_inhibitor_set_property;

	properties[PROP_SERVICE_ID] =
	        g_param_spec_string("service-id", "Service Id", NULL,
	                            NULL,
	                            G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE |
	                            G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties(object_class, LAST_INSTALLABLE_PROP, properties);

	/* Override Inhibitor properties */
	g_object_class_override_property(object_class, PROP_AVAILABLE, "available");
	properties[PROP_AVAILABLE] =
	        g_object_class_find_property(object_class, "available");

	g_object_class_override_property(object_class, PROP_INHIBITED, "inhibited");
	properties[PROP_INHIBITED] =
	        g_object_class_find_property(object_class, "inhibited");
}

static void
caphe_inhibitor_interface_init(CapheInhibitorInterface *iface)
{
	/* Implement methods */
	iface->inhibit       = caphe_dbus_inhibitor_inhibit;
	iface->uninhibit     = caphe_dbus_inhibitor_uninhibit;
	iface->get_available = caphe_dbus_inhibitor_get_available;
	iface->get_inhibited = caphe_dbus_inhibitor_get_inhibited;
}
