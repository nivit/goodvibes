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

#include "framework/gv-framework.h"

#ifdef CONSOLE_OUTPUT_ENABLED
#include "feat/gv-console-output.h"
#endif
#ifdef DBUS_SERVER_ENABLED
#include "feat/gv-dbus-server-native.h"
#include "feat/gv-dbus-server-mpris2.h"
#endif
#ifdef INHIBITOR_ENABLED
#include "feat/gv-inhibitor.h"
#endif
#ifdef HOTKEYS_ENABLED
#include "feat/gv-hotkeys.h"
#endif
#ifdef NOTIFICATIONS_ENABLED
#include "feat/gv-notifications.h"
#endif

#define MAX_FEATURES 12

static GvFeature *features[MAX_FEATURES];

GvFeature *
gv_feat_find(const gchar *name_to_find)
{
	guint i;

	for (i = 0; i < MAX_FEATURES; i++) {
		GvFeature *feature;
		const gchar *name;

		feature = features[i];
		if (feature == NULL)
			break;

		name = gv_feature_get_name(feature);
		if (!g_strcmp0(name, name_to_find))
			return feature;
	}

	return NULL;
}

void
gv_feat_cleanup(void)
{
	guint i;

	for (i = 0; i < MAX_FEATURES; i++) {
		GvFeature *feature;

		feature = features[i];
		if (feature == NULL)
			break;

		gv_framework_errorables_remove(feature);
		g_object_unref(feature);

		g_assert_null(features[i]);
	}
}

void
gv_feat_init(void)
{
	GvFeature *feature;
	guint i;

	/* Create every feature enabled at compile-time */
	i = 0;

	/* Core related features */

#ifdef CONSOLE_OUTPUT_ENABLED
	feature = gv_console_output_new();
	features[i++] = feature;

#endif
#ifdef DBUS_SERVER_ENABLED
	feature = gv_dbus_server_native_new();
	features[i++] = feature;

	feature = gv_dbus_server_mpris2_new();
	features[i++] = feature;
#endif
#ifdef INHIBITOR_ENABLED
	feature = gv_inhibitor_new();
	features[i++] = feature;
	gv_framework_errorables_append(feature);
#endif

	/* Ui related features */

#ifdef HOTKEYS_ENABLED
	feature = gv_hotkeys_new();
	features[i++] = feature;
	gv_framework_errorables_append(feature);
#endif
#ifdef NOTIFICATIONS_ENABLED
	feature = gv_notifications_new();
	features[i++] = feature;
#endif

	/* Additional work */

	for (i = 0; i < MAX_FEATURES; i++) {
		feature = features[i];
		if (feature == NULL)
			break;

		INFO("Feature compiled in: '%s'", gv_feature_get_name(feature));

		g_object_add_weak_pointer(G_OBJECT(features[i]),
		                          (gpointer *) &features[i]);
	}

	/* Ensure features are not overflowing */

	g_assert(i < MAX_FEATURES);
}
