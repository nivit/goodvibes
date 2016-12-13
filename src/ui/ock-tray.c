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

#include <math.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "additions/gtk.h"
#include "additions/glib-object.h"

#include "libgszn/gszn.h"

#include "framework/log.h"
#include "framework/ock-framework.h"

#include "core/ock-core.h"

#include "ui/global.h"
#include "ui/ock-ui-enum-types.h"
#include "ui/ock-main-menu.h"
#include "ui/ock-main-window.h"
#include "ui/ock-tray.h"

#define ICON_MIN_SIZE 16

/*
 * Properties
 */

#define DEFAULT_MIDDLE_CLICK_ACTION "toggle"
#define DEFAULT_SCROLL_ACTION       "station"

enum {
	/* Reserved */
	PROP_0,
	/* Properties */
	PROP_MIDDLE_CLICK_ACTION,
	PROP_SCROLL_ACTION,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * GObject definitions
 */

struct _OckTrayPrivate {
	/* Properties */
	OckTrayMiddleClickAction middle_click_action;
	OckTrayScrollAction      scroll_action;
	/* Right-click menu */
	GtkWidget        *main_menu;
	/* Status icon */
	GtkStatusIcon    *status_icon;
	guint             status_icon_size;
};

typedef struct _OckTrayPrivate OckTrayPrivate;

struct _OckTray {
	/* Parent instance structure */
	GObject         parent_instance;
	/* Private data */
	OckTrayPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(OckTray, ock_tray, G_TYPE_OBJECT)

/*
 * Helpers
 */

static void
ock_tray_update_icon_pixbuf(OckTray *self)
{
	OckTrayPrivate *priv = self->priv;
	GtkStatusIcon *status_icon = priv->status_icon;

	gtk_status_icon_set_from_icon_name(status_icon, PACKAGE_NAME);
}

static void
ock_tray_update_icon_tooltip(OckTray *self)
{
	OckTrayPrivate *priv = self->priv;
	GtkStatusIcon *status_icon = priv->status_icon;
	OckPlayer *player = ock_core_player;
	OckPlayerState player_state;
	const gchar *player_state_str;
	guint player_volume;
	gboolean player_muted;
	gchar *player_str;
	OckStation *station;
	gchar *station_str;
	OckMetadata *metadata;
	gchar *metadata_str;
	gchar *tooltip;

	/* Player */
	player_state = ock_player_get_state(player);
	player_volume = ock_player_get_volume(player);
	player_muted = ock_player_get_mute(player);

	switch (player_state) {
	case OCK_PLAYER_STATE_STOPPED:
		player_state_str = _("stopped");
		break;
	case OCK_PLAYER_STATE_CONNECTING:
		player_state_str = _("connecting");
		break;
	case OCK_PLAYER_STATE_BUFFERING:
		player_state_str = _("buffering");
		break;
	case OCK_PLAYER_STATE_PLAYING:
		player_state_str = _("playing");
		break;
	default:
		player_state_str = _("unknown state");
		break;
	}

	if (player_muted)
		player_str = g_strdup_printf("<b>" PACKAGE_LONG_NAME "</b> (%s, %s)",
		                             player_state_str, _("muted"));
	else
		player_str = g_strdup_printf("<b>" PACKAGE_LONG_NAME "</b> (%s, %s %u%%)",
		                             player_state_str, _("vol."), player_volume);

	/* Current station */
	station = ock_player_get_station(player);
	if (station)
		station_str = ock_station_make_name(station, TRUE);
	else
		station_str = g_strdup_printf("<i>%s</i>", _("No station"));

	/* Metadata */
	metadata = ock_player_get_metadata(player);
	if (metadata)
		metadata_str = ock_metadata_make_title_artist(metadata, TRUE);
	else
		metadata_str = g_strdup_printf("<i>%s</i>", _("No metadata"));

	/* Set the tooltip */
	tooltip = g_strdup_printf("%s\n%s\n%s",
	                          player_str,
	                          station_str,
	                          metadata_str);

	gtk_status_icon_set_tooltip_markup(status_icon, tooltip);

	/* Free */
	g_free(player_str);
	g_free(station_str);
	g_free(metadata_str);
	g_free(tooltip);
}

static void
ock_tray_update_icon(OckTray *self)
{
	/* Update icon */
	ock_tray_update_icon_pixbuf(self);

	/* Update tooltip */
	ock_tray_update_icon_tooltip(self);

	/* Set visible */
	gtk_status_icon_set_visible(self->priv->status_icon, TRUE);
}

/*
 * GtkStatusIcon signal handlers
 */

static void
on_activate(GtkStatusIcon *status_icon G_GNUC_UNUSED,
            OckTray       *self G_GNUC_UNUSED)
{
	GtkWindow *window = GTK_WINDOW(ock_ui_main_window);

	if (gtk_window_is_active(window))
		gtk_window_close(window);
	else
		gtk_window_present(window);
}

static void
on_popup_menu(GtkStatusIcon *status_icon,
              guint          button,
              guint          activate_time,
              OckTray       *self)
{
	OckTrayPrivate *priv = self->priv;
	GtkMenu *menu;

	/* First time, we need to create the menu */
	if (priv->main_menu == NULL) {
		priv->main_menu = ock_main_menu_new();
		g_object_ref_sink(priv->main_menu);
	}

	menu = GTK_MENU(priv->main_menu);

#if GTK_CHECK_VERSION(3,22,0)
	(void) status_icon;
	(void) button;
	(void) activate_time;
	gtk_menu_popup_at_pointer(menu, NULL);
#else
	gtk_menu_popup(menu,
	               NULL,
	               NULL,
	               gtk_status_icon_position_menu,
	               status_icon,
	               button,
	               activate_time);
#endif
}

static gboolean
on_button_release_event(GtkStatusIcon  *status_icon G_GNUC_UNUSED,
                        GdkEventButton *event,
                        OckTray        *self G_GNUC_UNUSED)
{
	OckTrayPrivate *priv = self->priv;
	OckPlayer *player = ock_core_player;

	/* Here we only handle the middle-click */
	if (event->button != 2)
		return FALSE;

	switch (priv->middle_click_action) {
	case OCK_TRAY_MIDDLE_CLICK_ACTION_TOGGLE:
		ock_player_toggle(player);
		break;
	case OCK_TRAY_MIDDLE_CLICK_ACTION_MUTE:
		ock_player_toggle_mute(player);
		break;
	default:
		CRITICAL("Unhandled middle-click action: %d",
		         priv->middle_click_action);
		break;
	}

	return FALSE;
}

static gboolean
on_scroll_event(GtkStatusIcon  *status_icon G_GNUC_UNUSED,
                GdkEventScroll *event,
                OckTray        *self G_GNUC_UNUSED)
{
	OckTrayPrivate *priv = self->priv;
	OckPlayer *player = ock_core_player;

	switch (priv->scroll_action) {
	case OCK_TRAY_SCROLL_ACTION_STATION:
		switch (event->direction) {
		case GDK_SCROLL_DOWN:
			ock_player_next(player);
			break;
		case GDK_SCROLL_UP:
			ock_player_prev(player);
			break;
		default:
			break;
		}
		break;
	case OCK_TRAY_SCROLL_ACTION_VOLUME:
		switch (event->direction) {
		case GDK_SCROLL_DOWN:
			ock_player_lower_volume(player);
			break;
		case GDK_SCROLL_UP:
			ock_player_raise_volume(player);
			break;
		default:
			break;
		}
		break;
	default:
		CRITICAL("Unhandled scroll action: %d", priv->scroll_action);
		break;
	}

	return FALSE;
}

static gboolean
on_size_changed(GtkStatusIcon *status_icon G_GNUC_UNUSED,
                gint           size,
                OckTray       *self G_GNUC_UNUSED)
{
	DEBUG("Tray icon size is now %d", size);

	ock_tray_update_icon(self);

	return FALSE;
}

/*
 * Overcooked signal handlers
 */

static void
on_player_notify(OckPlayer  *player,
                 GParamSpec *pspec,
                 OckTray    *self)
{
	const gchar *property_name = g_param_spec_get_name(pspec);

	TRACE("%p, %s, %p", player, property_name, self);

	if (!g_strcmp0(property_name, "state") ||
	    !g_strcmp0(property_name, "volume") ||
	    !g_strcmp0(property_name, "mute") ||
	    !g_strcmp0(property_name, "station") ||
	    !g_strcmp0(property_name, "metadata")) {
		ock_tray_update_icon(self);
		return;
	}
}

/*
 * Property accessors
 */

OckTrayMiddleClickAction
ock_tray_get_middle_click_action(OckTray *self)
{
	return self->priv->middle_click_action;
}

void
ock_tray_set_middle_click_action(OckTray *self, OckTrayMiddleClickAction action)
{
	OckTrayPrivate *priv = self->priv;

	if (priv->middle_click_action == action)
		return;

	priv->middle_click_action = action;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MIDDLE_CLICK_ACTION]);
}

OckTrayScrollAction
ock_tray_get_scroll_action(OckTray *self)
{
	return self->priv->scroll_action;
}

void
ock_tray_set_scroll_action(OckTray *self, OckTrayScrollAction action)
{
	OckTrayPrivate *priv = self->priv;

	if (priv->scroll_action == action)
		return;

	priv->scroll_action = action;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_SCROLL_ACTION]);
}

static void
ock_tray_get_property(GObject    *object,
                      guint       property_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
	OckTray *self = OCK_TRAY(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_MIDDLE_CLICK_ACTION:
		g_value_set_enum(value, ock_tray_get_middle_click_action(self));
		break;
	case PROP_SCROLL_ACTION:
		g_value_set_enum(value, ock_tray_get_scroll_action(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
ock_tray_set_property(GObject      *object,
                      guint         property_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
	OckTray *self = OCK_TRAY(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_MIDDLE_CLICK_ACTION:
		ock_tray_set_middle_click_action(self, g_value_get_enum(value));
		break;
	case PROP_SCROLL_ACTION:
		ock_tray_set_scroll_action(self, g_value_get_enum(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Public methods
 */

OckTray *
ock_tray_new(void)
{
	return g_object_new(OCK_TYPE_TRAY, NULL);
}

/*
 * GValue transform functions
 */

static void
value_transform_enum_string(const GValue *src_value,
                            GValue *dest_value)
{
	GEnumClass *enum_class;
	GEnumValue *enum_value;

	enum_class = g_type_class_ref(G_VALUE_TYPE(src_value));
	enum_value = g_enum_get_value(enum_class, g_value_get_enum(src_value));

	if (enum_value)
		g_value_set_static_string(dest_value, enum_value->value_nick);
	else {
		/* Assume zero holds the invalid value */
		enum_value = g_enum_get_value(enum_class, 0);
		g_value_set_static_string(dest_value, enum_value->value_nick);
	}

	g_type_class_unref(enum_class);
}

static void
value_transform_string_enum(const GValue *src_value,
                            GValue *dest_value)
{
	GEnumClass *enum_class;
	GEnumValue *enum_value;

	enum_class = g_type_class_ref(G_VALUE_TYPE(dest_value));
	enum_value = g_enum_get_value_by_nick(enum_class, g_value_get_string(src_value));

	if (enum_value)
		g_value_set_enum(dest_value, enum_value->value);
	else
		/* Assume zero holds the invalid value */
		g_value_set_enum(dest_value, 0);

	g_type_class_unref(enum_class);
}

/*
 * GObject methods
 */

static void
ock_tray_finalize(GObject *object)
{
	OckTray *self = OCK_TRAY(object);
	OckTrayPrivate *priv = self->priv;
	OckPlayer *player = ock_core_player;

	TRACE("%p", object);

	/* Destroy menu */
	if (priv->main_menu)
		g_object_unref(priv->main_menu);

	/* Disconnect signal handlers */
	g_signal_handlers_disconnect_by_data(player, self);

	/* Unref the status icon */
	g_object_unref(priv->status_icon);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_tray, object);
}

static void
ock_tray_constructed(GObject *object)
{
	OckTray *self = OCK_TRAY(object);
	OckTrayPrivate *priv = self->priv;
	OckPlayer *player = ock_core_player;
	GtkStatusIcon *status_icon;

	/* Create the tray icon */
	status_icon = gtk_status_icon_new();

	/* Connect signal handlers */
	g_signal_connect(status_icon, "activate",             /* Left click */
	                 G_CALLBACK(on_activate), self);
	g_signal_connect(status_icon, "popup-menu",           /* Right click */
	                 G_CALLBACK(on_popup_menu), self);
	g_signal_connect(status_icon, "button-release-event", /* Middle click */
	                 G_CALLBACK(on_button_release_event), self);
	g_signal_connect(status_icon, "scroll_event",         /* Mouse scroll */
	                 G_CALLBACK(on_scroll_event), self);
	g_signal_connect(status_icon, "size-changed",         /* Change of size */
	                 G_CALLBACK(on_size_changed), self);

	/* Save to private data */
	priv->status_icon = status_icon;
	priv->status_icon_size = ICON_MIN_SIZE;

	/* Connect core signal handlers */
	g_signal_connect(player, "notify", G_CALLBACK(on_player_notify), self);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_tray, object);
}

static void
ock_tray_init(OckTray *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_tray_get_instance_private(self);
}

static void
ock_tray_class_init(OckTrayClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_tray_finalize;
	object_class->constructed = ock_tray_constructed;

	/* Properties */
	object_class->get_property = ock_tray_get_property;
	object_class->set_property = ock_tray_set_property;

	properties[PROP_MIDDLE_CLICK_ACTION] =
	        g_param_spec_enum("middle-click-action", "Middle Click Action", NULL,
	                          OCK_TRAY_MIDDLE_CLICK_ACTION_ENUM_TYPE,
	                          OCK_TRAY_MIDDLE_CLICK_ACTION_TOGGLE,
	                          OCK_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

	properties[PROP_SCROLL_ACTION] =
	        g_param_spec_enum("scroll-action", "Scroll Action", NULL,
	                          OCK_TRAY_SCROLL_ACTION_ENUM_TYPE,
	                          OCK_TRAY_SCROLL_ACTION_STATION,
	                          OCK_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

	g_object_class_install_properties(object_class, PROP_N, properties);

	/* Register transform function */
	g_value_register_transform_func(OCK_TRAY_MIDDLE_CLICK_ACTION_ENUM_TYPE,
	                                G_TYPE_STRING,
	                                value_transform_enum_string);
	g_value_register_transform_func(G_TYPE_STRING,
	                                OCK_TRAY_MIDDLE_CLICK_ACTION_ENUM_TYPE,
	                                value_transform_string_enum);
	g_value_register_transform_func(OCK_TRAY_SCROLL_ACTION_ENUM_TYPE,
	                                G_TYPE_STRING,
	                                value_transform_enum_string);
	g_value_register_transform_func(G_TYPE_STRING,
	                                OCK_TRAY_SCROLL_ACTION_ENUM_TYPE,
	                                value_transform_string_enum);
}
