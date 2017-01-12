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
#include <keybinder.h>

#include "framework/gv-framework.h"

#include "core/gv-core.h"

#include "feat/gv-hotkeys.h"

/*
 * GObject definitions
 */

struct _GvHotkeys {
	/* Parent instance structure */
	GvFeature parent_instance;
};

G_DEFINE_TYPE_WITH_CODE(GvHotkeys, gv_hotkeys, GV_TYPE_FEATURE,
                        G_IMPLEMENT_INTERFACE(GV_TYPE_ERRORABLE, NULL))

/*
 * Signal handlers & callbacks
 */

static void
on_key_pressed(const gchar *keystring, void *data G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	DEBUG("Key '%s' pressed", keystring);

	if (!g_strcmp0(keystring, "XF86AudioPlay"))
		gv_player_play(player);
	else if (!g_strcmp0(keystring, "XF86AudioStop"))
		gv_player_stop(player);
	else if (!g_strcmp0(keystring, "XF86AudioPause"))
		gv_player_toggle(player);
	else if (!g_strcmp0(keystring, "XF86AudioPrev"))
		gv_player_prev(player);
	else if (!g_strcmp0(keystring, "XF86AudioNext"))
		gv_player_next(player);
	else
		WARNING("Unhandled key '%s'", keystring);
}

/*
 * Private methods
 */

struct _Binding {
	const gchar *key;
	gboolean     bound;
};

typedef struct _Binding Binding;

static Binding bindings[] = {
	{ "XF86AudioPlay",  FALSE },
	{ "XF86AudioPause", FALSE },
	{ "XF86AudioStop",  FALSE },
	{ "XF86AudioPrev",  FALSE },
	{ "XF86AudioNext",  FALSE },
	{ NULL,             FALSE }
};

static void
gv_hotkeys_unbind(GvHotkeys *self G_GNUC_UNUSED)
{
	Binding *binding;

	for (binding = bindings; binding->key != NULL; binding++) {
		if (binding->bound == TRUE) {
			keybinder_unbind(binding->key, on_key_pressed);
			binding->bound = FALSE;
		}
	}

	INFO("Multimedia keys undound");
}

static void
gv_hotkeys_bind(GvHotkeys *self)
{
	Binding *binding;
	GString *unbound_keys;

	unbound_keys = NULL;

	for (binding = bindings; binding->key != NULL; binding++) {
		const gchar *key;
		gboolean bound;

		key = binding->key;
		bound = keybinder_bind(key, on_key_pressed, self);
		if (bound == FALSE) {
			if (unbound_keys == NULL)
				unbound_keys = g_string_new(key);
			else
				g_string_append_printf(unbound_keys, ", %s", key);
		}

		binding->bound = bound;
	}

	if (unbound_keys) {
		gchar *text;

		text = g_string_free(unbound_keys, FALSE);

		INFO("Failed to bind the following keys: %s", text);
		gv_errorable_emit_error_printf
		(GV_ERRORABLE(self), "%s: \n%s",
		 _("Failed to bind the following keys"), text);

		g_free(text);
	} else {
		INFO("Multimedia keys successfully bound");
	}
}

/*
 * Feature methods
 */

static void
gv_hotkeys_disable(GvFeature *feature)
{
	/* Unbind keys */
	gv_hotkeys_unbind(GV_HOTKEYS(feature));

	/* Chain up */
	GV_FEATURE_CHAINUP_DISABLE(gv_hotkeys, feature);
}

static void
gv_hotkeys_enable(GvFeature *feature)
{
	/* Chain up */
	GV_FEATURE_CHAINUP_ENABLE(gv_hotkeys, feature);

	/* Unbind keys */
	gv_hotkeys_bind(GV_HOTKEYS(feature));
}

/*
 * GObject methods
 */

static void
gv_hotkeys_finalize(GObject *object)
{
	TRACE("%p", object);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(gv_hotkeys, object);
}

static void
gv_hotkeys_constructed(GObject *object)
{
	TRACE("%p", object);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(gv_hotkeys, object);
}

static void
gv_hotkeys_init(GvHotkeys *self)
{
	TRACE("%p", self);
}

static void
gv_hotkeys_class_init(GvHotkeysClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	GvFeatureClass *feature_class = GV_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = gv_hotkeys_finalize;
	object_class->constructed = gv_hotkeys_constructed;

	/* Override GvFeature methods */
	feature_class->enable = gv_hotkeys_enable;
	feature_class->disable = gv_hotkeys_disable;

	/* Init keybinder */
	keybinder_init();
}
