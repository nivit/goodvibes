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

#include <math.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "additions/gtk.h"
#include "additions/glib-object.h"

#include "libgszn/gszn.h"

#include "framework/log.h"
#include "framework/gv-framework.h"

#include "core/gv-core.h"

#include "ui/gv-ui-helpers.h"
#include "ui/gv-ui-enum-types.h"
#include "ui/gv-main-window.h"
#include "ui/gv-tray.h"

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
	PROP_MAIN_WINDOW,
	PROP_MIDDLE_CLICK_ACTION,
	PROP_SCROLL_ACTION,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * GObject definitions
 */

struct _GvTrayPrivate {
	/* Properties */
	GtkWindow               *main_window;
	GvTrayMiddleClickAction  middle_click_action;
	GvTrayScrollAction       scroll_action;
	/* Right-click menu */
	GtkWidget        *popup_menu;
	/* Status icon */
	GtkStatusIcon    *status_icon;
	guint             status_icon_size;
};

typedef struct _GvTrayPrivate GvTrayPrivate;

struct _GvTray {
	/* Parent instance structure */
	GObject         parent_instance;
	/* Private data */
	GvTrayPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(GvTray, gv_tray, G_TYPE_OBJECT)

/*
 * Helpers
 */

static void
gv_tray_update_icon_pixbuf(GvTray *self)
{
	GvTrayPrivate *priv = self->priv;
	GtkStatusIcon *status_icon = priv->status_icon;

	gtk_status_icon_set_from_icon_name(status_icon, PACKAGE_NAME);
}

static void
gv_tray_update_icon_tooltip(GvTray *self)
{
	GvTrayPrivate *priv = self->priv;
	GtkStatusIcon *status_icon = priv->status_icon;
	GvPlayer *player = gv_core_player;
	GvPlayerState player_state;
	const gchar *player_state_str;
	guint player_volume;
	gboolean player_muted;
	gchar *player_str;
	GvStation *station;
	gchar *station_str;
	GvMetadata *metadata;
	gchar *metadata_str;
	gchar *tooltip;

	/* Player */
	player_state = gv_player_get_state(player);
	player_volume = gv_player_get_volume(player);
	player_muted = gv_player_get_mute(player);

	switch (player_state) {
	case GV_PLAYER_STATE_STOPPED:
		player_state_str = _("stopped");
		break;
	case GV_PLAYER_STATE_CONNECTING:
		player_state_str = _("connecting");
		break;
	case GV_PLAYER_STATE_BUFFERING:
		player_state_str = _("buffering");
		break;
	case GV_PLAYER_STATE_PLAYING:
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
	station = gv_player_get_station(player);
	if (station)
		station_str = gv_station_make_name(station, TRUE);
	else
		station_str = g_strdup_printf("<i>%s</i>", _("No station"));

	/* Metadata */
	metadata = gv_player_get_metadata(player);
	if (metadata)
		metadata_str = gv_metadata_make_title_artist(metadata, TRUE);
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
gv_tray_update_icon(GvTray *self)
{
	/* Update icon */
	gv_tray_update_icon_pixbuf(self);

	/* Update tooltip */
	gv_tray_update_icon_tooltip(self);

	/* Set visible */
	gtk_status_icon_set_visible(self->priv->status_icon, TRUE);
}

/*
 * GtkStatusIcon signal handlers
 */

static void
on_activate(GtkStatusIcon *status_icon G_GNUC_UNUSED,
            GvTray       *self G_GNUC_UNUSED)
{
	GvTrayPrivate *priv = self->priv;
	GtkWindow *window = priv->main_window;

	if (gtk_window_is_active(window))
		gtk_window_close(window);
	else
		gtk_window_present(window);
}

static void
on_popup_menu(GtkStatusIcon *status_icon,
              guint          button,
              guint          activate_time,
              GvTray       *self)
{
	GvTrayPrivate *priv = self->priv;
	GtkMenu *menu = GTK_MENU(priv->popup_menu);

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
                        GvTray        *self G_GNUC_UNUSED)
{
	GvTrayPrivate *priv = self->priv;
	GvPlayer *player = gv_core_player;

	/* Here we only handle the middle-click */
	if (event->button != 2)
		return FALSE;

	switch (priv->middle_click_action) {
	case GV_TRAY_MIDDLE_CLICK_ACTION_TOGGLE:
		gv_player_toggle(player);
		break;
	case GV_TRAY_MIDDLE_CLICK_ACTION_MUTE:
		gv_player_toggle_mute(player);
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
                GvTray        *self G_GNUC_UNUSED)
{
	GvTrayPrivate *priv = self->priv;
	GvPlayer *player = gv_core_player;

	switch (priv->scroll_action) {
	case GV_TRAY_SCROLL_ACTION_STATION:
		switch (event->direction) {
		case GDK_SCROLL_DOWN:
			gv_player_next(player);
			break;
		case GDK_SCROLL_UP:
			gv_player_prev(player);
			break;
		default:
			break;
		}
		break;
	case GV_TRAY_SCROLL_ACTION_VOLUME:
		switch (event->direction) {
		case GDK_SCROLL_DOWN:
			gv_player_lower_volume(player);
			break;
		case GDK_SCROLL_UP:
			gv_player_raise_volume(player);
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
                GvTray       *self G_GNUC_UNUSED)
{
	DEBUG("Tray icon size is now %d", size);

	gv_tray_update_icon(self);

	return FALSE;
}

/*
 * Goodvibes signal handlers
 */

static void
on_player_notify(GvPlayer  *player,
                 GParamSpec *pspec,
                 GvTray    *self)
{
	const gchar *property_name = g_param_spec_get_name(pspec);

	TRACE("%p, %s, %p", player, property_name, self);

	if (!g_strcmp0(property_name, "state") ||
	    !g_strcmp0(property_name, "volume") ||
	    !g_strcmp0(property_name, "mute") ||
	    !g_strcmp0(property_name, "station") ||
	    !g_strcmp0(property_name, "metadata")) {
		gv_tray_update_icon(self);
		return;
	}
}

/*
 * Property accessors
 */

static void
gv_tray_set_main_window(GvTray *self, GtkWindow *main_window)
{
	GvTrayPrivate *priv = self->priv;

	/* Construct-only property */
	g_assert(priv->main_window == NULL);
	priv->main_window = g_object_ref(main_window);
}

GvTrayMiddleClickAction
gv_tray_get_middle_click_action(GvTray *self)
{
	return self->priv->middle_click_action;
}

void
gv_tray_set_middle_click_action(GvTray *self, GvTrayMiddleClickAction action)
{
	GvTrayPrivate *priv = self->priv;

	if (priv->middle_click_action == action)
		return;

	priv->middle_click_action = action;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MIDDLE_CLICK_ACTION]);
}

GvTrayScrollAction
gv_tray_get_scroll_action(GvTray *self)
{
	return self->priv->scroll_action;
}

void
gv_tray_set_scroll_action(GvTray *self, GvTrayScrollAction action)
{
	GvTrayPrivate *priv = self->priv;

	if (priv->scroll_action == action)
		return;

	priv->scroll_action = action;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_SCROLL_ACTION]);
}

static void
gv_tray_get_property(GObject    *object,
                     guint       property_id,
                     GValue     *value,
                     GParamSpec *pspec)
{
	GvTray *self = GV_TRAY(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_MIDDLE_CLICK_ACTION:
		g_value_set_enum(value, gv_tray_get_middle_click_action(self));
		break;
	case PROP_SCROLL_ACTION:
		g_value_set_enum(value, gv_tray_get_scroll_action(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
gv_tray_set_property(GObject      *object,
                     guint         property_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
	GvTray *self = GV_TRAY(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_MAIN_WINDOW:
		gv_tray_set_main_window(self, g_value_get_object(value));
		break;
	case PROP_MIDDLE_CLICK_ACTION:
		gv_tray_set_middle_click_action(self, g_value_get_enum(value));
		break;
	case PROP_SCROLL_ACTION:
		gv_tray_set_scroll_action(self, g_value_get_enum(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Public methods
 */

GvTray *
gv_tray_new(GtkWindow *main_window)
{
	return g_object_new(GV_TYPE_TRAY,
	                    "main-window", main_window,
	                    NULL);
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
gv_tray_finalize(GObject *object)
{
	GvTray *self = GV_TRAY(object);
	GvTrayPrivate *priv = self->priv;
	GvPlayer *player = gv_core_player;

	TRACE("%p", object);

	/* Disconnect signal handlers */
	g_signal_handlers_disconnect_by_data(player, self);

	/* Release main window */
	g_object_unref(priv->main_window);

	/* Destroy popup menu */
	g_object_unref(priv->popup_menu);

	/* Unref the status icon */
	g_object_unref(priv->status_icon);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(gv_tray, object);
}

static GtkWidget *
popup_menu_build(GtkWindow *window)
{
	GtkApplication *app;
	GMenuModel *menu_model;

	/* Popup menu is created from the app menu of the application */
	app = gtk_window_get_application(window);
	if (app == NULL)
		return NULL;

	menu_model = gtk_application_get_app_menu(app);
	if (menu_model == NULL)
		return NULL;

	return gtk_menu_new_from_model(menu_model);
}

static void
gv_tray_constructed(GObject *object)
{
	GvTray *self = GV_TRAY(object);
	GvTrayPrivate *priv = self->priv;
	GvPlayer *player = gv_core_player;
	GtkStatusIcon *status_icon;
	GtkWidget *menu;

	/* Ensure construct-only properties have been set */
	g_assert(priv->main_window != NULL);

	/* Create the tray icon */
	status_icon = gtk_status_icon_new();

	/* Create the popup menu */
	menu = popup_menu_build(priv->main_window);
	g_assert(menu != NULL);

	/* Attach the popup menu to the main window */
	gtk_menu_attach_to_widget(GTK_MENU(menu), GTK_WIDGET(priv->main_window), NULL);

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
	priv->popup_menu = g_object_ref_sink(menu);
	priv->status_icon = status_icon;
	priv->status_icon_size = ICON_MIN_SIZE;

	/* Connect core signal handlers */
	g_signal_connect(player, "notify", G_CALLBACK(on_player_notify), self);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(gv_tray, object);
}

static void
gv_tray_init(GvTray *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = gv_tray_get_instance_private(self);
}

static void
gv_tray_class_init(GvTrayClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = gv_tray_finalize;
	object_class->constructed = gv_tray_constructed;

	/* Properties */
	object_class->get_property = gv_tray_get_property;
	object_class->set_property = gv_tray_set_property;

	properties[PROP_MAIN_WINDOW] =
	        g_param_spec_object("main-window", "Main Window", NULL,
	                            GTK_TYPE_WINDOW,
	                            GV_PARAM_DEFAULT_FLAGS | G_PARAM_READWRITE |
	                            G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_MIDDLE_CLICK_ACTION] =
	        g_param_spec_enum("middle-click-action", "Middle Click Action", NULL,
	                          GV_TRAY_MIDDLE_CLICK_ACTION_ENUM_TYPE,
	                          GV_TRAY_MIDDLE_CLICK_ACTION_TOGGLE,
	                          GV_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

	properties[PROP_SCROLL_ACTION] =
	        g_param_spec_enum("scroll-action", "Scroll Action", NULL,
	                          GV_TRAY_SCROLL_ACTION_ENUM_TYPE,
	                          GV_TRAY_SCROLL_ACTION_STATION,
	                          GV_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

	g_object_class_install_properties(object_class, PROP_N, properties);

	/* Register transform function */
	g_value_register_transform_func(GV_TRAY_MIDDLE_CLICK_ACTION_ENUM_TYPE,
	                                G_TYPE_STRING,
	                                value_transform_enum_string);
	g_value_register_transform_func(G_TYPE_STRING,
	                                GV_TRAY_MIDDLE_CLICK_ACTION_ENUM_TYPE,
	                                value_transform_string_enum);
	g_value_register_transform_func(GV_TRAY_SCROLL_ACTION_ENUM_TYPE,
	                                G_TYPE_STRING,
	                                value_transform_enum_string);
	g_value_register_transform_func(G_TYPE_STRING,
	                                GV_TRAY_SCROLL_ACTION_ENUM_TYPE,
	                                value_transform_string_enum);
}
