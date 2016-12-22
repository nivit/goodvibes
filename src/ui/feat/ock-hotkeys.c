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

#include <glib.h>
#include <glib-object.h>
#include <keybinder.h>

#include "framework/log.h"
#include "framework/ock-framework.h"

#include "core/ock-core.h"

#include "ui/feat/ock-hotkeys.h"

/*
 * GObject definitions
 */

struct _OckHotkeys {
	/* Parent instance structure */
	OckFeature parent_instance;
};

G_DEFINE_TYPE_WITH_CODE(OckHotkeys, ock_hotkeys, OCK_TYPE_FEATURE,
                        G_IMPLEMENT_INTERFACE(OCK_TYPE_ERRORABLE, NULL))

/*
 * Signal handlers & callbacks
 */

static void
on_key_pressed(const gchar *keystring, void *data G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;

	DEBUG("Key '%s' pressed", keystring);

	if (!g_strcmp0(keystring, "XF86AudioPlay"))
		ock_player_play(player);
	else if (!g_strcmp0(keystring, "XF86AudioStop"))
		ock_player_stop(player);
	else if (!g_strcmp0(keystring, "XF86AudioPause"))
		ock_player_toggle(player);
	else if (!g_strcmp0(keystring, "XF86AudioPrev"))
		ock_player_prev(player);
	else if (!g_strcmp0(keystring, "XF86AudioNext"))
		ock_player_next(player);
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
ock_hotkeys_unbind(OckHotkeys *self G_GNUC_UNUSED)
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
ock_hotkeys_bind(OckHotkeys *self)
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
		ock_errorable_emit_error_printf
		(OCK_ERRORABLE(self), "%s: \n%s",
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
ock_hotkeys_disable(OckFeature *feature)
{
	/* Unbind keys */
	ock_hotkeys_unbind(OCK_HOTKEYS(feature));

	/* Chain up */
	OCK_FEATURE_CHAINUP_DISABLE(ock_hotkeys, feature);
}

static void
ock_hotkeys_enable(OckFeature *feature)
{
	/* Chain up */
	OCK_FEATURE_CHAINUP_ENABLE(ock_hotkeys, feature);

	/* Unbind keys */
	ock_hotkeys_bind(OCK_HOTKEYS(feature));
}

/*
 * GObject methods
 */

static void
ock_hotkeys_finalize(GObject *object)
{
	TRACE("%p", object);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_hotkeys, object);
}

static void
ock_hotkeys_constructed(GObject *object)
{
	TRACE("%p", object);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_hotkeys, object);
}

static void
ock_hotkeys_init(OckHotkeys *self)
{
	TRACE("%p", self);
}

static void
ock_hotkeys_class_init(OckHotkeysClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	OckFeatureClass *feature_class = OCK_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_hotkeys_finalize;
	object_class->constructed = ock_hotkeys_constructed;

	/* Override OckFeature methods */
	feature_class->enable = ock_hotkeys_enable;
	feature_class->disable = ock_hotkeys_disable;

	/* Init keybinder */
	keybinder_init();
}
