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
#include <libnotify/notify.h>

#include "additions/glib.h"

#include "libgszn/gszn.h"

#include "framework/gv-framework.h"

#include "core/gv-core.h"

#include "feat/gv-notifications.h"

//#define ICON "audio-x-generic"
#define ICON PACKAGE_NAME

/*
 * Properties
 */

#define DEFAULT_TIMEOUT_ENABLED FALSE
#define MIN_TIMEOUT_SECONDS     1
#define MAX_TIMEOUT_SECONDS     10
#define DEFAULT_TIMEOUT_SECONDS 5

enum {
	/* Reserved */
	PROP_0,
	/* Properties */
	PROP_TIMEOUT_ENABLED,
	PROP_TIMEOUT_SECONDS,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * GObject definitions
 */

struct _GvNotificationsPrivate {
	/* Properties */
	gboolean timeout_enabled;
	guint    timeout_ms;
	/* Notifications */
	NotifyNotification *notif_station;
	NotifyNotification *notif_metadata;
	NotifyNotification *notif_error;
};

typedef struct _GvNotificationsPrivate GvNotificationsPrivate;

struct _GvNotifications {
	/* Parent instance structure */
	GvFeature               parent_instance;
	/* Private data */
	GvNotificationsPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(GvNotifications, gv_notifications, GV_TYPE_FEATURE)

/*
 * Helpers
 */

static NotifyNotification *
make_notification(NotifyUrgency urgency, gint timeout_ms)
{
	NotifyNotification *notif;

	notif = notify_notification_new("", NULL, NULL);
	notify_notification_set_timeout(notif, timeout_ms);
	notify_notification_set_urgency(notif, urgency);

	return notif;
}

static gboolean
update_notification_station(NotifyNotification *notif, GvStation *station)
{
	const gchar *str;
	gchar *text;

	if (station == NULL)
		return FALSE;

	str = gv_station_get_name(station);
	if (str) {
		text = g_strdup_printf("Playing %s", str);
	} else {
		str = gv_station_get_uri(station);
		text = g_strdup_printf("Playing <%s>", str);
	}

	notify_notification_update(notif, text, NULL, ICON);

	g_free(text);

	return TRUE;
}

static gboolean
update_notification_metadata(NotifyNotification *notif, GvMetadata *metadata)
{
	const gchar *artist;
	const gchar *title;
	const gchar *album;
	const gchar *year;
	const gchar *genre;
	gchar *album_year;
	gchar *text;

	if (metadata == NULL)
		return FALSE;

	artist = gv_metadata_get_artist(metadata);
	title = gv_metadata_get_title(metadata);
	album = gv_metadata_get_album(metadata);
	year = gv_metadata_get_year(metadata);
	genre = gv_metadata_get_genre(metadata);

	/* If there's only the 'title' field, don't bother being clever,
	 * just display it as it. Actually, most radios fill only this field,
	 * and put everything in (title + artist + some more stuff).
	 */
	if (title && !artist && !album && !year && !genre) {
		notify_notification_update(notif, title, NULL, ICON);
		return TRUE;
	}

	/* Otherwise, each existing field is displayed on a line */
	if (title == NULL)
		title = "(Unknown title)";

	album_year = gv_metadata_make_album_year(metadata, FALSE);
	text = g_strjoin_null("\n", 4, title, artist, album_year, genre);

	notify_notification_update(notif, text, NULL, ICON);

	g_free(text);
	g_free(album_year);

	return TRUE;
}

static gboolean
update_notification_error(NotifyNotification *notif, const gchar *error_string)
{
	notify_notification_update(notif, PACKAGE_LONG_NAME, error_string, "dialog-error");

	return TRUE;
}

/*
 * Signal handlers & callbacks
 */

static void
on_player_notify(GvPlayer        *player,
                 GParamSpec       *pspec,
                 GvNotifications *self)
{
	GvNotificationsPrivate *priv = self->priv;
	const gchar *property_name = g_param_spec_get_name(pspec);
	NotifyNotification *notif = NULL;
	gboolean must_notify = FALSE;
	GError *error = NULL;

	/* Check what changed, and update notification if needed */
	if (!g_strcmp0(property_name, "state")) {
		GvPlayerState state;

		notif = priv->notif_station;
		state = gv_player_get_state(player);

		if (state == GV_PLAYER_STATE_PLAYING) {
			GvStation *station;

			station = gv_player_get_station(player);
			must_notify = update_notification_station(notif, station);
		}
	} else if (!g_strcmp0(property_name, "metadata")) {
		GvMetadata *metadata;

		notif = priv->notif_metadata;
		metadata = gv_player_get_metadata(player);
		must_notify = update_notification_metadata(notif, metadata);
	}

	/* There might be nothing to notify */
	if (notif == NULL || must_notify == FALSE)
		return;

	/* Show notification */
	if (notify_notification_show(notif, &error) != TRUE) {
		CRITICAL("Could not send notification: %s", error->message);
		g_error_free(error);
	}
}

static void
on_errorable_error(GvErrorable *errorable G_GNUC_UNUSED, const gchar *error_string,
                   GvNotifications *self)
{
	GvNotificationsPrivate *priv = self->priv;
	NotifyNotification *notif;
	GError *error = NULL;

	notif = priv->notif_error;

	/* Update notification */
	update_notification_error(notif, error_string);

	/* Show notification */
	if (notify_notification_show(notif, &error) != TRUE) {
		CRITICAL("Could not send notification: %s", error->message);
		g_error_free(error);
	}
}

/*
 * Property accessors
 */

static gboolean
gv_notifications_get_timeout_enabled(GvNotifications *self)
{
	return self->priv->timeout_enabled;
}

static void
gv_notifications_set_timeout_enabled(GvNotifications *self, gboolean enabled)
{
	GvNotificationsPrivate *priv = self->priv;
	gint timeout_ms;

	/* Bail out if needed */
	if (priv->timeout_enabled == enabled)
		return;

	/* Set to existing notifications that care about it */
	if (enabled == TRUE)
		timeout_ms = self->priv->timeout_ms;
	else
		timeout_ms = NOTIFY_EXPIRES_DEFAULT;

	if (priv->notif_station)
		notify_notification_set_timeout(priv->notif_station, timeout_ms);

	if (priv->notif_metadata)
		notify_notification_set_timeout(priv->notif_metadata, timeout_ms);

	/* Save and notify */
	priv->timeout_enabled = enabled;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_TIMEOUT_ENABLED]);
}

static gint
gv_notifications_get_timeout_seconds(GvNotifications *self)
{
	return self->priv->timeout_ms / 1000;
}

static void
gv_notifications_set_timeout_seconds(GvNotifications *self, guint timeout)
{
	GvNotificationsPrivate *priv = self->priv;

	/* No need to validate the incoming value, this function is private, it's
	 * only invoked by set_property(), where the value was validated already.
	 */

	/* Incoming value in seconds, we want to store it in ms for convenience */
	timeout *= 1000;

	/* Bail out if needed */
	if (priv->timeout_ms == timeout)
		return;

	/* Set to existing notifications */
	if (priv->notif_station && priv->timeout_enabled)
		notify_notification_set_timeout(priv->notif_station, timeout);

	if (priv->notif_metadata && priv->timeout_enabled)
		notify_notification_set_timeout(priv->notif_metadata, timeout);

	/* Save and notify */
	priv->timeout_ms = timeout;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_TIMEOUT_SECONDS]);
}

static void
gv_notifications_get_property(GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
	GvNotifications *self = GV_NOTIFICATIONS(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_TIMEOUT_ENABLED:
		g_value_set_boolean(value, gv_notifications_get_timeout_enabled(self));
		break;
	case PROP_TIMEOUT_SECONDS:
		g_value_set_uint(value, gv_notifications_get_timeout_seconds(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
gv_notifications_set_property(GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
	GvNotifications *self = GV_NOTIFICATIONS(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_TIMEOUT_ENABLED:
		gv_notifications_set_timeout_enabled(self, g_value_get_boolean(value));
		break;
	case PROP_TIMEOUT_SECONDS:
		gv_notifications_set_timeout_seconds(self, g_value_get_uint(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Feature methods
 */

static void
gv_notifications_disable(GvFeature *feature)
{
	GvNotificationsPrivate *priv = GV_NOTIFICATIONS(feature)->priv;
	GvPlayer *player = gv_core_player;

	/* Signal handlers */
	g_signal_handlers_disconnect_list_by_data(gv_framework_errorable_list, feature);
	g_signal_handlers_disconnect_by_data(player, feature);

	/* Unref notifications */
	g_object_unref(priv->notif_station);
	priv->notif_station = NULL;
	g_object_unref(priv->notif_metadata);
	priv->notif_metadata = NULL;
	g_object_unref(priv->notif_error);
	priv->notif_error = NULL;

	/* Cleanup libnotify */
	if (notify_is_initted() == TRUE)
		notify_uninit();

	/* Chain up */
	GV_FEATURE_CHAINUP_DISABLE(gv_notifications, feature);
}

static void
gv_notifications_enable(GvFeature *feature)
{
	GvNotificationsPrivate *priv = GV_NOTIFICATIONS(feature)->priv;
	GvPlayer *player = gv_core_player;

	/* Chain up */
	GV_FEATURE_CHAINUP_ENABLE(gv_notifications, feature);

	/* Init libnotify */
	g_assert(notify_is_initted() == FALSE);
	if (notify_init(PACKAGE_CAMEL_NAME) == FALSE)
		CRITICAL("Failed to initialize libnotify");

	/* Create notifications */
	priv->notif_station = make_notification
	                      (NOTIFY_URGENCY_LOW,
	                       priv->timeout_enabled ? (gint) priv->timeout_ms : -1);
	priv->notif_metadata = make_notification
	                       (NOTIFY_URGENCY_LOW,
	                        priv->timeout_enabled ? (gint) priv->timeout_ms : -1);
	priv->notif_error = make_notification
	                    (NOTIFY_URGENCY_NORMAL, 5 * 1000);

	/* Signal handlers */
	g_signal_connect(player, "notify", G_CALLBACK(on_player_notify), feature);
	g_signal_connect_list(gv_framework_errorable_list, "error",
	                      G_CALLBACK(on_errorable_error), feature);
}

/*
 * GObject methods
 */

static void
gv_notifications_init(GvNotifications *self)
{
	GvNotificationsPrivate *priv;

	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = gv_notifications_get_instance_private(self);

	/* Initialize properties */
	priv = self->priv;
	priv->timeout_enabled = DEFAULT_TIMEOUT_ENABLED;
	priv->timeout_ms = DEFAULT_TIMEOUT_SECONDS * 1000;
}

static void
gv_notifications_class_init(GvNotificationsClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	GvFeatureClass *feature_class = GV_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override GvFeature methods */
	feature_class->enable  = gv_notifications_enable;
	feature_class->disable = gv_notifications_disable;

	/* Properties */
	object_class->get_property = gv_notifications_get_property;
	object_class->set_property = gv_notifications_set_property;

	properties[PROP_TIMEOUT_ENABLED] =
	        g_param_spec_boolean("timeout-enabled", "Timeout Enabled",
	                             "Whether to use a custom timeout for the notifications",
	                             DEFAULT_TIMEOUT_ENABLED,
	                             GV_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                             G_PARAM_READWRITE);

	properties[PROP_TIMEOUT_SECONDS] =
	        g_param_spec_uint("timeout-seconds", "Timeout in seconds",
	                          "How long the notifications should be displayed",
	                          MIN_TIMEOUT_SECONDS,
	                          MAX_TIMEOUT_SECONDS,
	                          DEFAULT_TIMEOUT_SECONDS,
	                          GV_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                          G_PARAM_READWRITE);

	g_object_class_install_properties(object_class, PROP_N, properties);
}
