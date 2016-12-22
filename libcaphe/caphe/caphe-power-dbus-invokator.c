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
#include "caphe-dbus-invokator.h"
#include "caphe-power-dbus-invokator.h"

/*
 * Debug macros
 */

#define debug(fmt, ...) \
	g_debug("[PowerManagement]: " fmt, ##__VA_ARGS__)

#define warning(fmt, ...)       \
	g_warning("[PowerManagement]: " fmt, ##__VA_ARGS__)

/*
 * GObject definitions
 */

struct _CaphePowerDbusInvokatorPrivate {
	guint32 cookie;
};

typedef struct _CaphePowerDbusInvokatorPrivate CaphePowerDbusInvokatorPrivate;

struct _CaphePowerDbusInvokator {
	/* Parent instance structure */
	CapheDbusInvokator           parent_instance;
	/* Private data */
	CaphePowerDbusInvokatorPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(CaphePowerDbusInvokator, caphe_power_dbus_invokator,
                           CAPHE_TYPE_DBUS_INVOKATOR)

/*
 * Callbacks
 */

static void
on_proxy_uninhibit_call_finished(GDBusProxy *proxy,
                                 GAsyncResult *result,
                                 CaphePowerDbusInvokator *self)
{
	CaphePowerDbusInvokatorPrivate *priv = self->priv;
	GError *error = NULL;

	priv->cookie = 0;

	g_dbus_proxy_call_finish(proxy, result, &error);
	if (error) {
		warning("%s", error->message);
		g_error_free(error);
		goto notify_inhibited;
	}

	debug("Uninhibition successful");

notify_inhibited:
	g_object_notify(G_OBJECT(self), "inhibited");
}

static void
on_proxy_inhibit_call_finished(GDBusProxy *proxy,
                               GAsyncResult *result,
                               CaphePowerDbusInvokator *self)
{
	CaphePowerDbusInvokatorPrivate *priv = self->priv;
	GError *error = NULL;
	GVariant *variant;
	guint32 cookie;

	variant = g_dbus_proxy_call_finish(proxy, result, &error);
	if (error) {
		warning("%s", error->message);
		g_error_free(error);
		goto notify_inhibited;
	}

	cookie = 0;
	g_variant_get(variant, "(u)", &cookie);
	g_variant_unref(variant);

	if (cookie == 0) {
		warning("Failed to unpack variant (cookie is zero)");
		goto notify_inhibited;
	}

	priv->cookie = cookie;
	debug("Inhibition successful (cookie: %u)", cookie);

notify_inhibited:
	g_object_notify(G_OBJECT(self), "inhibited");
}

/*
 * DbusInvokator methods
 */

static gboolean
caphe_power_dbus_invokator_is_inhibited(CapheDbusInvokator *dbus_invokator)
{
	CaphePowerDbusInvokator *self = CAPHE_POWER_DBUS_INVOKATOR(dbus_invokator);
	CaphePowerDbusInvokatorPrivate *priv = self->priv;

	return priv->cookie != 0;
}

static void
caphe_power_dbus_invokator_uninhibit(CapheDbusInvokator *dbus_invokator)
{
	CaphePowerDbusInvokator *self = CAPHE_POWER_DBUS_INVOKATOR(dbus_invokator);
	CaphePowerDbusInvokatorPrivate *priv = self->priv;
	GDBusProxy *proxy = caphe_dbus_invokator_get_proxy(dbus_invokator);

	if (priv->cookie == 0) {
		warning("Not inhibited (no cookie)");
		return;
	}

	g_dbus_proxy_call(proxy,
	                  "UnInhibit",
	                  g_variant_new("(u)", priv->cookie),
	                  G_DBUS_CALL_FLAGS_NO_AUTO_START,
	                  -1,
	                  NULL,
	                  (GAsyncReadyCallback) on_proxy_uninhibit_call_finished,
	                  self);
}

static void
caphe_power_dbus_invokator_inhibit(CapheDbusInvokator *dbus_invokator,
                                   const gchar *application, const gchar *reason)
{
	CaphePowerDbusInvokator *self = CAPHE_POWER_DBUS_INVOKATOR(dbus_invokator);
	CaphePowerDbusInvokatorPrivate *priv = self->priv;
	GDBusProxy *proxy = caphe_dbus_invokator_get_proxy(dbus_invokator);

	if (priv->cookie != 0) {
		warning("Already inhibited (cookie: %u)", priv->cookie);
		return;
	}

	g_dbus_proxy_call(proxy,
	                  "Inhibit",
	                  g_variant_new("(ss)", application, reason),
	                  G_DBUS_CALL_FLAGS_NO_AUTO_START,
	                  -1,
	                  NULL,
	                  (GAsyncReadyCallback) on_proxy_inhibit_call_finished,
	                  self);
}

/*
 * GObject methods
 */

static void
caphe_power_dbus_invokator_init(CaphePowerDbusInvokator *self)
{
	CaphePowerDbusInvokatorPrivate *priv =
	        caphe_power_dbus_invokator_get_instance_private(self);

	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = priv;
}

static void
caphe_power_dbus_invokator_class_init(CaphePowerDbusInvokatorClass *class)
{
	CapheDbusInvokatorClass *dbus_invokator_class = CAPHE_DBUS_INVOKATOR_CLASS(class);

	TRACE("%p", class);

	/* Implement methods */
	dbus_invokator_class->inhibit      = caphe_power_dbus_invokator_inhibit;
	dbus_invokator_class->uninhibit    = caphe_power_dbus_invokator_uninhibit;
	dbus_invokator_class->is_inhibited = caphe_power_dbus_invokator_is_inhibited;
}
