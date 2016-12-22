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
#include <gio/gunixfdlist.h>

#include "caphe-trace.h"
#include "caphe-dbus-invokator.h"
#include "caphe-login-dbus-invokator.h"

/*
 * Debug macros
 */

#define debug(fmt, ...) \
	g_debug("[login1]: " fmt, ##__VA_ARGS__)

#define warning(fmt, ...)       \
	g_warning("[login1]: " fmt, ##__VA_ARGS__)

/*
 * GObject definitions
 */

struct _CapheLoginDbusInvokatorPrivate {
	gint32 fd;
};

typedef struct _CapheLoginDbusInvokatorPrivate CapheLoginDbusInvokatorPrivate;

struct _CapheLoginDbusInvokator {
	/* Parent instance structure */
	CapheDbusInvokator              parent_instance;
	/* Private data */
	CapheLoginDbusInvokatorPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(CapheLoginDbusInvokator, caphe_login_dbus_invokator,
                           CAPHE_TYPE_DBUS_INVOKATOR)

/*
 * Callbacks
 */

gboolean
when_idle_notify_inhibited(CapheLoginDbusInvokator *self)
{
	debug("Uninhibition successful");
	g_object_notify(G_OBJECT(self), "inhibited");

	return G_SOURCE_REMOVE;
}


static void
on_proxy_inhibit_call_finished(GDBusProxy *proxy,
                               GAsyncResult *result,
                               CapheLoginDbusInvokator *self)
{
	CapheLoginDbusInvokatorPrivate *priv = self->priv;
	GUnixFDList *fd_list = NULL;
	GError *error = NULL;
	GVariant *variant;
	gint32 fd_index;
	gint fd;

	variant = g_dbus_proxy_call_with_unix_fd_list_finish(proxy, &fd_list, result, &error);
	if (error) {
		warning("%s", error->message);
		goto cleanup;
	}

	fd_index = -1;
	g_variant_get(variant, "(h)", &fd_index);
	if (fd_index < 0) {
		warning("Failed to unpack variant (fd index < 0)");
		goto cleanup;
	}

	fd = g_unix_fd_list_get(fd_list, fd_index, &error);
	if (error) {
		warning("Failed to get fd[%d]: %s", fd_index, error->message);
		goto cleanup;
	}

	priv->fd = fd;
	debug("Inhibition successful (fd: %d)", fd);

cleanup:
	if (error)
		g_error_free(error);

	if (variant)
		g_variant_unref(variant);

	if (fd_list)
		g_object_unref(fd_list);

	g_object_notify(G_OBJECT(self), "inhibited");
}

/*
 * DbusInvokator methods
 */

static gboolean
caphe_login_dbus_invokator_is_inhibited(CapheDbusInvokator *dbus_invokator)
{
	CapheLoginDbusInvokator *self = CAPHE_LOGIN_DBUS_INVOKATOR(dbus_invokator);
	CapheLoginDbusInvokatorPrivate *priv = self->priv;

	return priv->fd != -1;
}

static void
caphe_login_dbus_invokator_uninhibit(CapheDbusInvokator *dbus_invokator)
{
	CapheLoginDbusInvokator *self = CAPHE_LOGIN_DBUS_INVOKATOR(dbus_invokator);
	CapheLoginDbusInvokatorPrivate *priv = self->priv;

	if (priv->fd < 0) {
		warning("Already uninhibited (no fd)");
		return;
	}

	close(priv->fd);
	priv->fd = -1;

	/* Signal emission is done async to ensure a consistent behavior with
	 * other dbus invokators.
	 */
	g_idle_add((GSourceFunc) when_idle_notify_inhibited, self);
}

static void
caphe_login_dbus_invokator_inhibit(CapheDbusInvokator *dbus_invokator,
                                   const gchar *application, const gchar *reason)
{
	CapheLoginDbusInvokator *self = CAPHE_LOGIN_DBUS_INVOKATOR(dbus_invokator);
	CapheLoginDbusInvokatorPrivate *priv = self->priv;
	GDBusProxy *proxy = caphe_dbus_invokator_get_proxy(dbus_invokator);

	if (priv->fd >= 0) {
		warning("Already inhibited (fd: %d)", priv->fd);
		return;
	}

	/*
	 * Documentation at:
	 * https://www.freedesktop.org/wiki/Software/systemd/inhibit/
	 */
	g_dbus_proxy_call_with_unix_fd_list
	(proxy,
	 "Inhibit",
	 g_variant_new("(ssss)", "sleep", application, reason, "block"),
	 G_DBUS_CALL_FLAGS_NO_AUTO_START,
	 -1,
	 NULL, /* fd_list */
	 NULL, /* cancellable */
	 (GAsyncReadyCallback) on_proxy_inhibit_call_finished,
	 self);
}

/*
 * GObject methods
 */

static void
caphe_login_dbus_invokator_init(CapheLoginDbusInvokator *self)
{
	CapheLoginDbusInvokatorPrivate *priv =
	        caphe_login_dbus_invokator_get_instance_private(self);

	TRACE("%p", self);

	/* Initialize inhibit file descriptor */
	priv->fd = -1;

	/* Initialize private pointer */
	self->priv = priv;
}

static void
caphe_login_dbus_invokator_class_init(CapheLoginDbusInvokatorClass *class)
{
	CapheDbusInvokatorClass *dbus_invokator_class = CAPHE_DBUS_INVOKATOR_CLASS(class);

	TRACE("%p", class);

	/* Implement methods */
	dbus_invokator_class->inhibit      = caphe_login_dbus_invokator_inhibit;
	dbus_invokator_class->uninhibit    = caphe_login_dbus_invokator_uninhibit;
	dbus_invokator_class->is_inhibited = caphe_login_dbus_invokator_is_inhibited;
}
