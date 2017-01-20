/*
 * Goodvibes Radio Player
 *
 * Copyright (C) 2015-2017 Arnaud Rebillout
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

#include "additions/glib.h"

#include "framework/gv-framework.h"

#include "core/gv-core.h"

#include "feat/gv-notifications.h"

#define ICON_PACKAGE PACKAGE_NAME /* "audio-x-generic" is also suitable */
#define ICON_ERROR   "dialog-error"

/*
 * GObject definitions
 */

struct _GvNotificationsPrivate {
	/* Notifications */
	GNotification *notif_station;
	GNotification *notif_metadata;
	GNotification *notif_error;
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

static GNotification *
make_notification(const gchar *title, const gchar *icon_name, GNotificationPriority priority)
{
	GNotification *notif;
	GIcon *icon;

	notif = g_notification_new(title);

	icon = g_themed_icon_new(icon_name);
	g_notification_set_icon(notif, icon);
	g_object_unref(icon);

	g_notification_set_priority(notif, priority);

	return notif;
}

static gboolean
update_notification_station(GNotification *notif, GvStation *station)
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

	g_notification_set_body(notif, text);

	g_free(text);

	return TRUE;
}

static gboolean
update_notification_metadata(GNotification *notif, GvMetadata *metadata)
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

	/* If there's only the 'title' field, don't bother being smart,
	 * just display it as it. Actually, most radios fill only this field,
	 * and put everything in it (title + artist + some more stuff).
	 */
	if (title && !artist && !album && !year && !genre) {
		g_notification_set_body(notif, title);
		return TRUE;
	}

	/* Otherwise, each existing field is displayed on a line */
	if (title == NULL)
		title = "(Unknown title)";

	album_year = gv_metadata_make_album_year(metadata, FALSE);
	text = g_strjoin_null("\n", 4, title, artist, album_year, genre);

	g_notification_set_body(notif, text);

	g_free(text);
	g_free(album_year);

	return TRUE;
}

static gboolean
update_notification_error(GNotification *notif, const gchar *error_string)
{
	// TODO Add application name ?
	g_notification_set_body(notif, error_string);

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
	GApplication *app = gv_core_application;

	if (!g_strcmp0(property_name, "state")) {
		GNotification *notif = priv->notif_station;
		gboolean must_notify = FALSE;
		GvPlayerState state;
		GvStation *station;

		state = gv_player_get_state(player);
		if (state != GV_PLAYER_STATE_PLAYING)
			return;

		station = gv_player_get_station(player);
		must_notify = update_notification_station(notif, station);
		if (must_notify == FALSE)
			return;

		g_application_send_notification(app, "station", notif);

	} else if (!g_strcmp0(property_name, "metadata")) {
		GNotification *notif = priv->notif_metadata;
		gboolean must_notify = FALSE;
		GvMetadata *metadata;

		metadata = gv_player_get_metadata(player);
		must_notify = update_notification_metadata(notif, metadata);
		if (must_notify == FALSE)
			return;

		g_application_send_notification(app, "metadata", notif);
	}
}

static void
on_errorable_error(GvErrorable *errorable G_GNUC_UNUSED, const gchar *error_string,
                   GvNotifications *self)
{
	GvNotificationsPrivate *priv = self->priv;
	GNotification *notif = priv->notif_error;
	GApplication *app = gv_core_application;

	update_notification_error(notif, error_string);
	g_application_send_notification(app, "error", notif);
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

	/* Create notifications */
	g_assert_null(priv->notif_station);
	priv->notif_station = make_notification(_("Playing Station"), ICON_PACKAGE,
	                                        G_NOTIFICATION_PRIORITY_NORMAL);
	g_assert_null(priv->notif_metadata);
	priv->notif_metadata = make_notification(_("New Metadata"), ICON_PACKAGE,
	                       G_NOTIFICATION_PRIORITY_NORMAL);
	g_assert_null(priv->notif_error);
	priv->notif_error = make_notification(_("Error"), ICON_ERROR,
	                                      G_NOTIFICATION_PRIORITY_NORMAL);

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
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = gv_notifications_get_instance_private(self);
}

static void
gv_notifications_class_init(GvNotificationsClass *class)
{
	GvFeatureClass *feature_class = GV_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override GvFeature methods */
	feature_class->enable  = gv_notifications_enable;
	feature_class->disable = gv_notifications_disable;
}
