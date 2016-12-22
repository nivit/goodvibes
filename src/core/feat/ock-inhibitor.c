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

#include <signal.h>

#include <glib.h>
#include <glib-object.h>

#include "caphe/caphe.h"

#include "additions/glib.h"

#include "framework/log.h"
#include "framework/ock-feature.h"

#include "core/ock-core.h"

#include "core/feat/ock-inhibitor.h"

/*
 * GObject definitions
 */

struct _OckInhibitorPrivate {
	guint when_timeout_id;
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
 * Signal handlers & callbacks
 */

static void
on_caphe_notify_inhibited(CapheMain    *caphe,
                          GParamSpec   *pspec G_GNUC_UNUSED,
                          OckInhibitor *self G_GNUC_UNUSED)
{
	gboolean inhibited = caphe_main_get_inhibited(caphe);
	const gchar *inhibitor_id = caphe_main_get_inhibitor_id(caphe);

	if (inhibited)
		INFO("Sleep inhibited (%s)", inhibitor_id);
	else
		INFO("Sleep uninhibited");
}

static gboolean
when_timeout_check_playback_status(OckInhibitor *self)
{
	OckInhibitorPrivate *priv = self->priv;
	OckPlayer *player = ock_core_player;
	OckPlayerState player_state = ock_player_get_state(player);

	/* Not interested about the transitional states */
	if (player_state == OCK_PLAYER_STATE_PLAYING)
		caphe_main_inhibit(caphe_get_default(), "Playing");
	else if (player_state == OCK_PLAYER_STATE_STOPPED)
		caphe_main_uninhibit(caphe_get_default());

	priv->when_timeout_id = 0;

	return G_SOURCE_REMOVE;
}

static void
on_player_notify_state(OckPlayer    *player,
                       GParamSpec   *pspec G_GNUC_UNUSED,
                       OckInhibitor *self)
{
	OckInhibitorPrivate *priv = self->priv;
	OckPlayerState player_state = ock_player_get_state(player);

	/* Not interested about the transitional states */
	if (player_state != OCK_PLAYER_STATE_PLAYING &&
	    player_state != OCK_PLAYER_STATE_STOPPED)
		return;

	/* Delay the actual inhibition, just in case the states are getting
	 * crazy and changing fast.
	 */
	if (priv->when_timeout_id > 0)
		g_source_remove(priv->when_timeout_id);

	priv->when_timeout_id = g_timeout_add_seconds
		(1, (GSourceFunc) when_timeout_check_playback_status, self);
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

	/* Remove pending operation */
	if (priv->when_timeout_id) {
		g_source_remove(priv->when_timeout_id);
		priv->when_timeout_id = 0;
	}

	/* Signal handlers */
	g_signal_handlers_disconnect_by_data(player, feature);

	/* Cleanup libcaphe */
	g_signal_handlers_disconnect_by_data(caphe_get_default(), self);
	caphe_cleanup();

	/* Chain up */
	OCK_FEATURE_CHAINUP_DISABLE(ock_inhibitor, feature);
}

static void
ock_inhibitor_enable(OckFeature *feature)
{
	OckInhibitor *self = OCK_INHIBITOR(feature);
	OckInhibitorPrivate *priv = self->priv;
	OckPlayer *player = ock_core_player;

	/* Chain up */
	OCK_FEATURE_CHAINUP_ENABLE(ock_inhibitor, feature);

	/* Init libcaphe */
	caphe_init(g_get_application_name());
	g_signal_connect(caphe_get_default(), "notify::inhibited",
	                 G_CALLBACK(on_caphe_notify_inhibited), self);

	/* Connect to signal handlers */
	g_signal_connect(player, "notify::state", G_CALLBACK(on_player_notify_state), self);

	/* Schedule a check for the current playback status */
	g_assert(priv->when_timeout_id == 0);
	priv->when_timeout_id = g_timeout_add_seconds
		(1, (GSourceFunc) when_timeout_check_playback_status, self);
}

/*
 * GObject methods
 */

static void
ock_inhibitor_finalize(GObject *object)
{
	TRACE("%p", object);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_inhibitor, object);
}

static void
ock_inhibitor_constructed(GObject *object)
{
	TRACE("%p", object);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_inhibitor, object);
}

static void
ock_inhibitor_init(OckInhibitor *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_inhibitor_get_instance_private(self);
}

static void
ock_inhibitor_class_init(OckInhibitorClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	OckFeatureClass *feature_class = OCK_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_inhibitor_finalize;
	object_class->constructed = ock_inhibitor_constructed;

	/* Override OckFeature methods */
	feature_class->enable = ock_inhibitor_enable;
	feature_class->disable = ock_inhibitor_disable;
}
