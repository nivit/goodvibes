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
#include <gtk/gtk.h>

#include "additions/glib-object.h"
#include "additions/gtk.h"

#include "libgszn/gszn.h"

#include "framework/log.h"
#include "framework/ock-framework.h"

#include "core/ock-core.h"

#include "ui/global.h"
#include "ui/ock-builder-helpers.h"
#include "ui/ock-main-window.h"
#include "ui/ock-stations-tree-view.h"

#define UI_FILE "ui/main-window.glade"

/* For debugging */
#define CLOSE_ON_FOCUS_OUT TRUE

/*
 * GObject definitions
 */

struct _OckMainWindowPrivate {
	/*
	 * Widgets
	 */

	/* Top-level */
	GtkWidget *window_vbox;
	/* Current status */
	GtkWidget *station_label;
	GtkWidget *status_label;
	/* Button box */
	GtkWidget *play_button;
	GtkWidget *prev_button;
	GtkWidget *next_button;
	GtkWidget *repeat_toggle_button;
	GtkWidget *shuffle_toggle_button;
	GtkWidget *volume_button;
	/* Stations */
	GtkWidget *stations_tree_view;

	/*
	 * Bindings
	 */

	GBinding *volume_binding;
};

typedef struct _OckMainWindowPrivate OckMainWindowPrivate;

struct _OckMainWindow {
	/* Parent instance structure */
	GtkWindow             parent_instance;
	/* Private data */
	OckMainWindowPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(OckMainWindow, ock_main_window, GTK_TYPE_WINDOW)

/*
 * Core Player signal handlers
 */

static void
set_station_label(GtkLabel *label, OckStation *station)
{
	const gchar *station_title;

	if (station)
		station_title = ock_station_get_name_or_uri(station);
	else
		station_title = "No station selected";

	gtk_label_set_text(label, station_title);
}

static void
set_status_label(GtkLabel *label, OckPlayerState state, OckMetadata *metadata)
{
	if (state != OCK_PLAYER_STATE_PLAYING || metadata == NULL) {
		const gchar *state_str;

		switch (state) {
		case OCK_PLAYER_STATE_PLAYING:
			state_str = "Playing";
			break;
		case OCK_PLAYER_STATE_CONNECTING:
			state_str = "Connecting...";
			break;
		case OCK_PLAYER_STATE_BUFFERING:
			state_str = "Buffering...";
			break;
		case OCK_PLAYER_STATE_STOPPED:
		default:
			state_str = "Stopped";
			break;
		}

		gtk_label_set_text(label, state_str);
	} else {
		gchar *artist_title;
		gchar *album_year;
		gchar *str;

		artist_title = ock_metadata_make_title_artist(metadata, FALSE);
		album_year = ock_metadata_make_album_year(metadata, FALSE);

		if (artist_title && album_year)
			str = g_strdup_printf("%s/n%s", artist_title, album_year);
		else if (artist_title)
			str = g_strdup(artist_title);
		else
			str = g_strdup("Playing");

		gtk_label_set_text(label, str);

		g_free(str);
		g_free(album_year);
		g_free(artist_title);
	}
}

static void
set_play_button(GtkButton *button, OckPlayerState state)
{
	GtkWidget *image;
	const gchar *icon_name;

	if (state == OCK_PLAYER_STATE_STOPPED)
		icon_name = "media-playback-start";
	else
		icon_name = "media-playback-stop";

	image = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(button, image);
}

static void
set_volume_button(GtkVolumeButton *volume_button, guint volume, gboolean mute)
{
	GtkScaleButton *scale_button = GTK_SCALE_BUTTON(volume_button);

	if (mute)
		gtk_scale_button_set_value(scale_button, 0);
	else
		gtk_scale_button_set_value(scale_button, volume);
}

static void
on_player_notify(OckPlayer     *player,
                 GParamSpec    *pspec,
                 OckMainWindow *self)
{
	OckMainWindowPrivate *priv = self->priv;
	const gchar *property_name = g_param_spec_get_name(pspec);

	TRACE("%p, %s, %p", player, property_name, self);

	if (!g_strcmp0(property_name, "station")) {
		GtkLabel *label = GTK_LABEL(priv->station_label);
		OckStation *station = ock_player_get_station(player);

		set_station_label(label, station);

	} else if (!g_strcmp0(property_name, "state")) {
		GtkLabel *label = GTK_LABEL(priv->status_label);
		GtkButton *button = GTK_BUTTON(priv->play_button);
		OckPlayerState state = ock_player_get_state(player);
		OckMetadata *metadata = ock_player_get_metadata(player);

		set_status_label(label, state, metadata);
		set_play_button(button, state);

	} else if (!g_strcmp0(property_name, "metadata")) {
		GtkLabel *label = GTK_LABEL(priv->status_label);
		OckPlayerState state = ock_player_get_state(player);
		OckMetadata *metadata = ock_player_get_metadata(player);

		set_status_label(label, state, metadata);

	}  else if (!g_strcmp0(property_name, "mute")) {
		GtkVolumeButton *volume_button = GTK_VOLUME_BUTTON(priv->volume_button);
		guint volume = ock_player_get_volume(player);
		gboolean mute = ock_player_get_mute(player);

		g_binding_unbind(priv->volume_binding);
		set_volume_button(volume_button, volume, mute);
		priv->volume_binding = g_object_bind_property
		                       (player, "volume",
		                        volume_button, "value",
		                        G_BINDING_BIDIRECTIONAL);
	}
}

/*
 * Widget signal handlers
 */

static void
on_button_clicked(GtkButton *button, OckMainWindow *self)
{
	OckMainWindowPrivate *priv = self->priv;
	OckPlayer *player = ock_core_player;
	GtkWidget *widget = GTK_WIDGET(button);

	if (widget == priv->play_button) {
		ock_player_toggle(player);
	} else if (widget == priv->prev_button) {
		ock_player_prev(player);
	} else if (widget == priv->next_button) {
		ock_player_next(player);
	} else {
		CRITICAL("Unhandled button %p", button);
	}
}

/*
 * Window signal handlers
 */

static gboolean
on_window_key_press_event(GtkWindow     *window,
                          GdkEventKey   *event,
                          gpointer data G_GNUC_UNUSED)
{
	OckPlayer *player = ock_core_player;

	g_assert(event->type == GDK_KEY_PRESS);

	switch (event->keyval) {
	case GDK_KEY_Escape:
		/* Close window if 'Esc' key is pressed */
		gtk_window_close(window);
		break;

	case GDK_KEY_blank:
		/* Play/Stop when space is pressed */
		ock_player_toggle(player);
		break;

	default:
		break;
	}

	return FALSE;
}

static gboolean
on_window_focus_change(GtkWindow     *window,
                       GdkEventFocus *focus_event,
                       gpointer data G_GNUC_UNUSED)
{
	g_assert(focus_event->type == GDK_FOCUS_CHANGE);

	//DEBUG("Main window %s focus", focus_event->in ? "gained" : "lost");

	/* Close window if focus was lost */
	if (focus_event->in == FALSE && CLOSE_ON_FOCUS_OUT)
		gtk_window_close(window);

	return FALSE;
}

/*
 * Construct private methods
 */

static void
setup_adjustment(GtkAdjustment *adjustment, GObject *obj, const gchar *obj_prop)
{
	guint minimum, maximum;
	guint range;

	/* Get property bounds, and assign it to the adjustment */
	g_object_get_property_uint_bounds(obj, obj_prop, &minimum, &maximum);
	range = maximum - minimum;

	gtk_adjustment_set_lower(adjustment, minimum);
	gtk_adjustment_set_upper(adjustment, maximum);
	gtk_adjustment_set_step_increment(adjustment, range / 100);
	gtk_adjustment_set_page_increment(adjustment, range / 10);
}

static void
setup_action(const gchar *tooltip_text,
             GtkWidget *widget, const gchar *widget_signal,
             GCallback callback, gpointer data)
{
	/* Tooltip */
	if (tooltip_text)
		gtk_widget_set_tooltip_text(widget, tooltip_text);

	/* Signal handler */
	g_signal_connect(widget, widget_signal, callback, data);
}

static void
setup_setting(const gchar *tooltip_text,
              GtkWidget *widget, const gchar *widget_prop,
              GObject *obj, const gchar *obj_prop)
{
	/* Tooltip */
	if (tooltip_text)
		gtk_widget_set_tooltip_text(widget, tooltip_text);

	/* Binding: obj 'prop' <-> widget 'prop'
	 * Order matters, don't mix up source and target here...
	 */
	g_object_bind_property(obj, obj_prop, widget, widget_prop,
	                       G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
}

/*
 * Public methods
 */

void
ock_main_window_populate_stations(OckMainWindow *self)
{
	OckMainWindowPrivate *priv = self->priv;
	OckStationsTreeView *tree_view = OCK_STATIONS_TREE_VIEW(priv->stations_tree_view);

	ock_stations_tree_view_populate(tree_view);
}

GtkWidget *
ock_main_window_new(void)
{
	return g_object_new(OCK_TYPE_MAIN_WINDOW, NULL);
}

/*
 * Construct helpers
 */

static void
ock_main_window_populate_widgets(OckMainWindow *self)
{
	OckMainWindowPrivate *priv = self->priv;
	GtkBuilder *builder;
	gchar *uifile;

	/* Build the ui */
	ock_builder_load(UI_FILE, &builder, &uifile);
	DEBUG("Built from ui file '%s'", uifile);

	/* Save widget pointers */

	/* Top-level */
	GTK_BUILDER_SAVE_WIDGET(builder, priv, window_vbox);

	/* Current status */
	GTK_BUILDER_SAVE_WIDGET(builder, priv, station_label);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, status_label);

	/* Button box */
	GTK_BUILDER_SAVE_WIDGET(builder, priv, play_button);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, prev_button);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, next_button);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, repeat_toggle_button);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, shuffle_toggle_button);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, volume_button);

	/* Now create the stations tree view */
	priv->stations_tree_view = ock_stations_tree_view_new();
	gtk_widget_show_all(priv->stations_tree_view);
	gtk_box_pack_start(GTK_BOX(priv->window_vbox), priv->stations_tree_view,
	                   TRUE, TRUE, 0);


	/* Pack that within the window */
	gtk_container_add(GTK_CONTAINER(self), priv->window_vbox);

	/* Cleanup */
	g_object_unref(G_OBJECT(builder));
	g_free(uifile);
}

static void
ock_main_window_setup_widgets(OckMainWindow *self)
{
	OckMainWindowPrivate *priv = self->priv;
	GObject *player_obj = G_OBJECT(ock_core_player);

	/* Setup adjustments - must be done first, before setting widget values */
	setup_adjustment(gtk_scale_button_get_adjustment(GTK_SCALE_BUTTON(priv->volume_button)),
	                 player_obj, "volume");

	/*
	 * Setup settings and actions.
	 * These functions calls set the widgets tooltips. Additionally, they
	 * create the link between the widgets and program.
	 * - settings: the link is a binding between the gtk widget and
	 * an internal object.
	 * - actions: the link is a callback invoked when the widget is
	 * activated/clicked or whatever. This is a one-way link from the ui to
	 * the application, and additional work is needed if the widget should
	 * react to external changes.
	 */

	setup_action("Start/Stop playing the current station.",
	             priv->play_button, "clicked",
	             G_CALLBACK(on_button_clicked), self);

	setup_action("Jump to the previous station.",
	             priv->prev_button, "clicked",
	             G_CALLBACK(on_button_clicked), self);

	setup_action("Jump to the next station.",
	             priv->next_button, "clicked",
	             G_CALLBACK(on_button_clicked), self);

	setup_setting("Whether to loop on the station list. "
	              "Only affects the behaviour of the previous/next actions.",
	              priv->repeat_toggle_button, "active",
	              player_obj, "repeat");

	setup_setting("Whether to browse the station list in a random order. "
	              "Only affects the behaviour of the previous/next actions.",
	              priv->shuffle_toggle_button, "active",
	              player_obj, "shuffle");

	/* Volume button comes with automatic tooltip, so we just need to bind.
	 * Plus, we need to remember the binding because we do strange things with it.
	 */
	priv->volume_binding = g_object_bind_property
	                       (player_obj, "volume",
	                        priv->volume_button, "value",
	                        G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
}

static void
ock_main_window_setup_layout(OckMainWindow *self)
{
	OckMainWindowPrivate *priv = self->priv;

	g_object_set(priv->window_vbox,
	             "spacing", OCK_UI_ELEM_SPACING,
	             NULL);
}

static void
ock_main_window_configure(OckMainWindow *self)
{
	GtkWindow *window = GTK_WINDOW(self);

	/*
	 * Configure window
	 * Basically, we want the window to appear
	 * and behave as a popup window.
	 */

	/* Window appearance */
	gtk_window_set_decorated(window, FALSE);
	gtk_window_set_position(window, GTK_WIN_POS_MOUSE);

	/* We don't want the window to appear in pager or taskbar.
	 * This has an undesired effect though: the window may not
	 * have the focus when it's shown by the window manager.
	 * But read on...
	 */
	gtk_window_set_skip_pager_hint(window, TRUE);
	gtk_window_set_skip_taskbar_hint(window, TRUE);

	/* Setting the window modal seems to ensure that the window
	 * receives focus when shown by the window manager.
	 */
	gtk_window_set_modal(window, TRUE);

	/* We want the window to be hidden instead of destroyed when closed */
	g_signal_connect(window, "delete-event",
	                 G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	/* Handle keyboard focus changes, so that we can hide the
	 * window on 'focus-out-event'.
	 */
	g_signal_connect(window, "focus-in-event",
	                 G_CALLBACK(on_window_focus_change), NULL);
	g_signal_connect(window, "focus-out-event",
	                 G_CALLBACK(on_window_focus_change), NULL);

	/* Handle some keys */
	g_signal_connect(window, "key-press-event",
	                 G_CALLBACK(on_window_key_press_event), NULL);
}

/*
 * GObject methods
 */

static void
ock_main_window_finalize(GObject *object)
{
	OckMainWindow *self = OCK_MAIN_WINDOW(object);
	OckPlayer *player = ock_core_player;

	TRACE("%p", object);

	/* Disconnect core signal handlers */
	g_signal_handlers_disconnect_by_data(player, self);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_main_window, object);
}

static void
ock_main_window_constructed(GObject *object)
{
	OckMainWindow *self = OCK_MAIN_WINDOW(object);
	OckPlayer *player = ock_core_player;

	/* Build window */
	ock_main_window_populate_widgets(self);
	ock_main_window_setup_widgets(self);
	ock_main_window_setup_layout(self);

	/* Configure window */
	ock_main_window_configure(self);

	/* Connect core signal handlers */
	g_signal_connect(player, "notify",
	                 G_CALLBACK(on_player_notify), self);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_main_window, object);
}

static void
ock_main_window_init(OckMainWindow *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_main_window_get_instance_private(self);
}

static void
ock_main_window_class_init(OckMainWindowClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_main_window_finalize;
	object_class->constructed = ock_main_window_constructed;
}
