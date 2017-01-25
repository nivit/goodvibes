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

		g_object_unref(feature);
	}
}

void
gv_feat_init(void)
{
	GvFeature *feature;
	guint i = 0;

	/*
	 * Create every feature enabled at compile-time.
	 *
	 * Notice that some features don't really make sense if the program
	 * has been compiled without ui. However we don't bother about that
	 * here: all features are equals !
	 * The distinction between 'core' features and 'ui' features is done
	 * by the build system, see the `configure.ac` for more details.
	 */

	/*
	 * 'Notifications' features must come first, so that they're up
	 * and ready first, and can report errors from other features.
	 */

#ifdef CONSOLE_OUTPUT_ENABLED
	feature = gv_console_output_new();
	gv_framework_register(feature);
	features[i++] = feature;

#endif
#ifdef NOTIFICATIONS_ENABLED
	feature = gv_notifications_new();
	gv_framework_register(feature);
	features[i++] = feature;
#endif

	/* Now comes the rest of the features */

#ifdef DBUS_SERVER_ENABLED
	feature = gv_dbus_server_native_new();
	gv_framework_register(feature);
	features[i++] = feature;

	feature = gv_dbus_server_mpris2_new();
	gv_framework_register(feature);
	features[i++] = feature;
#endif
#ifdef INHIBITOR_ENABLED
	feature = gv_inhibitor_new();
	gv_framework_register(feature);
	features[i++] = feature;
#endif
#ifdef HOTKEYS_ENABLED
	feature = gv_hotkeys_new();
	gv_framework_register(feature);
	features[i++] = feature;
#endif

	/* Sum up the situation */

	for (i = 0; i < MAX_FEATURES; i++) {
		feature = features[i];
		if (feature == NULL)
			break;

		INFO("Feature compiled in: '%s'", gv_feature_get_name(feature));
	}

	/* Ensure features are not overflowing */

	g_assert(i < MAX_FEATURES);
}
