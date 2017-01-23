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

#include <signal.h>

#include <glib.h>
#include <glib-object.h>

#include "caphe/caphe.h"

#include "framework/gv-framework.h"

#include "core/gv-core.h"

#include "feat/gv-inhibitor.h"

/*
 * GObject definitions
 */

struct _GvInhibitorPrivate {
	guint when_timeout_id;
	gboolean error_emited;
};

typedef struct _GvInhibitorPrivate GvInhibitorPrivate;

struct _GvInhibitor {
	/* Parent instance structure */
	GvFeature           parent_instance;
	/* Private data */
	GvInhibitorPrivate *priv;
};

G_DEFINE_TYPE_WITH_CODE(GvInhibitor, gv_inhibitor, GV_TYPE_FEATURE,
                        G_ADD_PRIVATE(GvInhibitor)
                        G_IMPLEMENT_INTERFACE(GV_TYPE_ERRORABLE, NULL))

/*
 * Signal handlers & callbacks
 */

static void
on_caphe_inhibit_finished(CapheMain *caphe G_GNUC_UNUSED,
                          gboolean success,
                          GvInhibitor *self)
{
	GvInhibitorPrivate *priv = self->priv;

	/* In case of error, we notify the user, only for the first error.
	 * The following errors are silent.
	 */

	if (success)
		return;

	g_info("Failed to inhibit system sleep");

	if (priv->error_emited)
		return;

	gv_errorable_emit_error(GV_ERRORABLE(self), _("Failed to inhibit system sleep"));
	priv->error_emited = TRUE;
}

static void
on_caphe_notify_inhibited(CapheMain    *caphe,
                          GParamSpec   *pspec G_GNUC_UNUSED,
                          GvInhibitor *self G_GNUC_UNUSED)
{
	gboolean inhibited = caphe_main_get_inhibited(caphe);
	const gchar *inhibitor_id = caphe_main_get_inhibitor_id(caphe);

	if (inhibited)
		INFO("System sleep inhibited (%s)", inhibitor_id);
	else
		INFO("System sleep uninhibited");
}

static gboolean
when_timeout_check_playback_status(GvInhibitor *self)
{
	GvInhibitorPrivate *priv = self->priv;
	GvPlayer *player = gv_core_player;
	GvPlayerState player_state = gv_player_get_state(player);

	/* Not interested about the transitional states */
	if (player_state == GV_PLAYER_STATE_PLAYING)
		caphe_main_inhibit(caphe_get_default(), "Playing");
	else if (player_state == GV_PLAYER_STATE_STOPPED)
		caphe_main_uninhibit(caphe_get_default());

	priv->when_timeout_id = 0;

	return G_SOURCE_REMOVE;
}

static void
on_player_notify_state(GvPlayer    *player,
                       GParamSpec   *pspec G_GNUC_UNUSED,
                       GvInhibitor *self)
{
	GvInhibitorPrivate *priv = self->priv;
	GvPlayerState player_state = gv_player_get_state(player);

	/* Not interested about the transitional states */
	if (player_state != GV_PLAYER_STATE_PLAYING &&
	    player_state != GV_PLAYER_STATE_STOPPED)
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
gv_inhibitor_disable(GvFeature *feature)
{
	GvInhibitor *self = GV_INHIBITOR(feature);
	GvInhibitorPrivate *priv = self->priv;
	GvPlayer *player = gv_core_player;

	/* Remove pending operation */
	if (priv->when_timeout_id) {
		g_source_remove(priv->when_timeout_id);
		priv->when_timeout_id = 0;
	}

	/* Reset error emission */
	priv->error_emited = FALSE;

	/* Signal handlers */
	g_signal_handlers_disconnect_by_data(player, feature);

	/* Cleanup libcaphe */
	g_signal_handlers_disconnect_by_data(caphe_get_default(), self);
	caphe_cleanup();

	/* Chain up */
	GV_FEATURE_CHAINUP_DISABLE(gv_inhibitor, feature);
}

static void
gv_inhibitor_enable(GvFeature *feature)
{
	GvInhibitor *self = GV_INHIBITOR(feature);
	GvInhibitorPrivate *priv = self->priv;
	GvPlayer *player = gv_core_player;

	/* Chain up */
	GV_FEATURE_CHAINUP_ENABLE(gv_inhibitor, feature);

	/* Init libcaphe */
	caphe_init(g_get_application_name());
	g_signal_connect(caphe_get_default(), "notify::inhibited",
	                 G_CALLBACK(on_caphe_notify_inhibited), self);
	g_signal_connect(caphe_get_default(), "inhibit-finished",
	                 G_CALLBACK(on_caphe_inhibit_finished), self);

	/* Connect to signal handlers */
	g_signal_connect(player, "notify::state", G_CALLBACK(on_player_notify_state), self);

	/* Schedule a check for the current playback status */
	g_assert(priv->when_timeout_id == 0);
	priv->when_timeout_id = g_timeout_add_seconds
	                        (1, (GSourceFunc) when_timeout_check_playback_status, self);
}

/*
 * Public methods
 */

GvFeature *
gv_inhibitor_new(void)
{
	return gv_feature_new(GV_TYPE_INHIBITOR, "Inhibitor");
}

/*
 * GObject methods
 */

static void
gv_inhibitor_init(GvInhibitor *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = gv_inhibitor_get_instance_private(self);
}

static void
gv_inhibitor_class_init(GvInhibitorClass *class)
{
	GvFeatureClass *feature_class = GV_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override GvFeature methods */
	feature_class->enable = gv_inhibitor_enable;
	feature_class->disable = gv_inhibitor_disable;
}
