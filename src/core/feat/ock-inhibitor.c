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

/*
 * This code was mainly taken from:
 *   systemd-inhibit (systemd source tree: src/login/inhibit.c)
 * For other ways to inhibit sleep:
 *   caffeine-ng
 *   xfce4-power-manager
 */

// TODO This thing doesn't work ! Get rid of that.

#include <signal.h>

#include <glib.h>
#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include <glib-object.h>

#include "additions/glib.h"

#include "framework/log.h"
#include "framework/ock-feature.h"

#include "core/ock-core.h"

#include "core/feat/ock-inhibitor.h"

/*
 * GObject definitions
 */

struct _OckInhibitorPrivate {
	/* DBus connection */
	GDBusConnection *dbus_connection;
	/* File descriptor to inhibit sleep */
	gint             inhibit_fd;
};

typedef struct _OckInhibitorPrivate OckInhibitorPrivate;

struct _OckInhibitor {
	/* Parent instance structure */
	OckFeature           parent_instance;
	/* Private data */
	OckInhibitorPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(OckInhibitor, ock_inhibitor, OCK_TYPE_FEATURE)

/*
 * DBus callbacks
 */

static void ock_inhibitor_check_player_state(OckInhibitor *self, OckPlayer *player);

static void
on_bus_acquired(GObject      *source_object G_GNUC_UNUSED,
                GAsyncResult *result,
                gpointer      user_data)
{
	OckInhibitor *self = OCK_INHIBITOR(user_data);
	OckInhibitorPrivate *priv = self->priv;
	OckPlayer *player = ock_core_player;
	GError *error = NULL;

	g_assert(priv->dbus_connection == NULL);

	/* Get the dbus connection */
	priv->dbus_connection = g_bus_get_finish(result, &error);
	if (priv->dbus_connection == NULL) {
		/* Error is guaranteed to be set */
		WARNING("Failed to acquire dbus connection: %s", error->message);
		g_error_free(error);
		return;
	}
	DEBUG("Dbus connection acquired");

	/* Now we must check the player state, in case it's playing */
	ock_inhibitor_check_player_state(self, player);
}

static void
on_message_reply(GObject      *source_object,
                 GAsyncResult *result,
                 gpointer      user_data)
{
	OckInhibitor *self = OCK_INHIBITOR(user_data);
	OckInhibitorPrivate *priv = self->priv;
	GDBusConnection *connection = G_DBUS_CONNECTION(source_object);
	OckPlayer *player = ock_core_player;
	GError *error = NULL;
	GDBusMessage *reply;
	GUnixFDList *fd_list;
	gint fd;

	/* Get the message reply */
	reply = g_dbus_connection_send_message_with_reply_finish
	        (connection, result, &error);

	if (reply == NULL) {
		/* Error is guaranteed to be set */
		WARNING("Local error while sending message: %s", error->message);
		g_error_free(error);
		return;
	}

	if (g_dbus_message_get_message_type(reply) == G_DBUS_MESSAGE_TYPE_ERROR) {
		g_dbus_message_to_gerror(reply, &error);
		WARNING("Reply message is an error: %s", error->message);
		g_error_free(error);
		goto cleanup;
	}

	/* The message is supposed to hold a file descriptor */
	fd_list = g_dbus_message_get_unix_fd_list(reply);
	if (fd_list == NULL) {
		WARNING("Reply message has no file descriptors");
		goto cleanup;
	}

	fd = g_unix_fd_list_get(fd_list, 0, &error);
	if (error) {
		WARNING("Failed to get fd: %s", error->message);
		g_error_free(error);
		goto cleanup;
	}

	/* At this point, we succeeded to inhibit sleep.
	 * We must save the file descriptor, since we will close it
	 * when we want to uninhibit.
	 */
	DEBUG("Inhibit file descriptor: %d", fd);
	priv->inhibit_fd = fd;

	/* One last verification: ensure that music is still playing.
	 * It's possible (though unlikely) that music was stopped while
	 * we were waiting for the inhibitor file descriptor.
	 */
	ock_inhibitor_check_player_state(self, player);

	/* Cleanup before exiting */
cleanup:
	g_object_unref(reply);
}

/*
 * Private methods
 */

static void
ock_inhibitor_start(OckInhibitor *self)
{
	OckInhibitorPrivate *priv = self->priv;
	GDBusMessage *message;

	/* May be inhibited already */
	if (priv->inhibit_fd >= 0) {
		DEBUG("Inhibit file descriptor is already set");
		return;
	}

	/* Ensure we can access DBus */
	if (priv->dbus_connection == NULL) {
		DEBUG("Not connected to dbus");
		return;
	}

	/* Inhibit */
	DEBUG("Inhibiting sleep");

	/* Create the message to send to the login manager */
	message = g_dbus_message_new_method_call
	          ("org.freedesktop.login1",
	           "/org/freedesktop/login1",
	           "org.freedesktop.login1.Manager",
	           "Inhibit");

	g_dbus_message_set_body
	(message,
	 g_variant_new("(ssss)",
	               "sleep",
	               PACKAGE_NAME,
	               PACKAGE_CAMEL_NAME " is playing and prevents sleeping",
	               "block"));

	/* Send the message asynchronously */
	g_dbus_connection_send_message_with_reply
	(priv->dbus_connection,
	 message,
	 G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	 -1,
	 NULL, /* out_serial */
	 NULL, /* cancellable */
	 on_message_reply,
	 self);

	/* Cleanup */
	g_object_unref(message);
}

static void
ock_inhibitor_stop(OckInhibitor *self)
{
	OckInhibitorPrivate *priv = self->priv;

	/* May be unhinibited already */
	if (priv->inhibit_fd < 0) {
		DEBUG("Inhibit file descriptor is not set");
		return;
	}

	/* Uninhibit */
	DEBUG("Uninhibiting sleep");
	close(priv->inhibit_fd);
	priv->inhibit_fd = -1;
}

static void
ock_inhibitor_check_player_state(OckInhibitor *self, OckPlayer *player)
{
	OckPlayerState player_state;

	player_state = ock_player_get_state(player);

	switch (player_state) {
	case OCK_PLAYER_STATE_PLAYING:
		ock_inhibitor_start(self);
		break;
	case OCK_PLAYER_STATE_STOPPED:
		ock_inhibitor_stop(self);
		break;
	default:
		break;
	}
}

/*
 * Signal handlers & callbacks
 */

static void
on_player_notify_state(OckPlayer    *player,
                       GParamSpec   *pspec,
                       OckInhibitor *self)
{
	const gchar *property_name = g_param_spec_get_name(pspec);

	TRACE("%p, %s, %p", player, property_name, self);

	ock_inhibitor_check_player_state(self, player);
}

/*
 * Feature methods
 */

static void
ock_inhibitor_disable(OckFeature *feature)
{
	OckInhibitor *self = OCK_INHIBITOR(feature);
	OckInhibitorPrivate *priv = self->priv;
	OckPlayer *player = ock_core_player;

	/* Signal handlers */
	g_signal_handlers_disconnect_by_data(player, feature);

	/* Stop inhibitor */
	ock_inhibitor_stop(self);

	/* Unref the dbus connection */
	if (priv->dbus_connection) {
		g_object_unref(priv->dbus_connection);
		priv->dbus_connection = NULL;
	}

	/* Chain up */
	OCK_FEATURE_CHAINUP_DISABLE(ock_inhibitor, feature);
}

static void
ock_inhibitor_enable(OckFeature *feature)
{
	OckPlayer *player = ock_core_player;

	/* Chain up */
	OCK_FEATURE_CHAINUP_ENABLE(ock_inhibitor, feature);

	/* Acquire the connection to dbus */
	g_bus_get(G_BUS_TYPE_SYSTEM, NULL, on_bus_acquired, feature);

	/* Signal handlers */
	g_signal_connect(player, "notify::state", G_CALLBACK(on_player_notify_state), feature);
}

/*
 * GObject methods
 */

static void
ock_inhibitor_init(OckInhibitor *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_inhibitor_get_instance_private(self);

	/* Initialize inhibit file descriptor */
	self->priv->inhibit_fd = -1;
}

static void
ock_inhibitor_class_init(OckInhibitorClass *class)
{
	OckFeatureClass *feature_class = OCK_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override OckFeature methods */
	feature_class->enable = ock_inhibitor_enable;
	feature_class->disable = ock_inhibitor_disable;
}
