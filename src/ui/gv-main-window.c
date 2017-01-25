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
#include <gtk/gtk.h>

#include "additions/glib-object.h"
#include "additions/gtk.h"
#include "framework/gv-framework.h"
#include "core/gv-core.h"
#include "ui/gv-ui-internal.h"
#include "ui/gv-ui-helpers.h"
#include "ui/gv-stations-tree-view.h"

#include "ui/gv-main-window.h"

#define UI_FILE "main-window.glade"

/*
 * GObject definitions
 */

struct _GvMainWindowPrivate {
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

typedef struct _GvMainWindowPrivate GvMainWindowPrivate;

struct _GvMainWindow {
	/* Parent instance structure */
	GtkApplicationWindow  parent_instance;
	/* Private data */
	GvMainWindowPrivate  *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(GvMainWindow, gv_main_window, GTK_TYPE_APPLICATION_WINDOW)

/*
 * Core Player signal handlers
 */

static void
set_station_label(GtkLabel *label, GvStation *station)
{
	const gchar *station_title;

	if (station)
		station_title = gv_station_get_name_or_uri(station);
	else
		station_title = _("No station selected");

	gtk_label_set_text(label, station_title);
}

static void
set_status_label(GtkLabel *label, GvPlayerState state, GvMetadata *metadata)
{
	if (state != GV_PLAYER_STATE_PLAYING || metadata == NULL) {
		const gchar *state_str;

		switch (state) {
		case GV_PLAYER_STATE_PLAYING:
			state_str = _("Playing");
			break;
		case GV_PLAYER_STATE_CONNECTING:
			state_str = _("Connecting...");
			break;
		case GV_PLAYER_STATE_BUFFERING:
			state_str = _("Buffering...");
			break;
		case GV_PLAYER_STATE_STOPPED:
		default:
			state_str = _("Stopped");
			break;
		}

		gtk_label_set_text(label, state_str);
	} else {
		gchar *artist_title;
		gchar *album_year;
		gchar *str;

		artist_title = gv_metadata_make_title_artist(metadata, FALSE);
		album_year = gv_metadata_make_album_year(metadata, FALSE);

		if (artist_title && album_year)
			str = g_strdup_printf("%s/n%s", artist_title, album_year);
		else if (artist_title)
			str = g_strdup(artist_title);
		else
			str = g_strdup(_("Playing"));

		gtk_label_set_text(label, str);

		g_free(str);
		g_free(album_year);
		g_free(artist_title);
	}
}

static void
set_play_button(GtkButton *button, GvPlayerState state)
{
	GtkWidget *image;
	const gchar *icon_name;

	if (state == GV_PLAYER_STATE_STOPPED)
		icon_name = "media-playback-start-symbolic";
	else
		icon_name = "media-playback-stop-symbolic";

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
on_player_notify(GvPlayer     *player,
                 GParamSpec    *pspec,
                 GvMainWindow *self)
{
	GvMainWindowPrivate *priv = self->priv;
	const gchar *property_name = g_param_spec_get_name(pspec);

	TRACE("%p, %s, %p", player, property_name, self);

	if (!g_strcmp0(property_name, "station")) {
		GtkLabel *label = GTK_LABEL(priv->station_label);
		GvStation *station = gv_player_get_station(player);

		set_station_label(label, station);

	} else if (!g_strcmp0(property_name, "state")) {
		GtkLabel *label = GTK_LABEL(priv->status_label);
		GtkButton *button = GTK_BUTTON(priv->play_button);
		GvPlayerState state = gv_player_get_state(player);
		GvMetadata *metadata = gv_player_get_metadata(player);

		set_status_label(label, state, metadata);
		set_play_button(button, state);

	} else if (!g_strcmp0(property_name, "metadata")) {
		GtkLabel *label = GTK_LABEL(priv->status_label);
		GvPlayerState state = gv_player_get_state(player);
		GvMetadata *metadata = gv_player_get_metadata(player);

		set_status_label(label, state, metadata);

	}  else if (!g_strcmp0(property_name, "mute")) {
		GtkVolumeButton *volume_button = GTK_VOLUME_BUTTON(priv->volume_button);
		guint volume = gv_player_get_volume(player);
		gboolean mute = gv_player_get_mute(player);

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
on_button_clicked(GtkButton *button, GvMainWindow *self)
{
	GvMainWindowPrivate *priv = self->priv;
	GvPlayer *player = gv_core_player;
	GtkWidget *widget = GTK_WIDGET(button);

	if (widget == priv->play_button) {
		gv_player_toggle(player);
	} else if (widget == priv->prev_button) {
		gv_player_prev(player);
	} else if (widget == priv->next_button) {
		gv_player_next(player);
	} else {
		CRITICAL("Unhandled button %p", button);
	}
}

/*
 * Popup window signal handlers
 */

#define POPUP_WINDOW_CLOSE_ON_FOCUS_OUT TRUE

static gboolean
on_popup_window_focus_change(GtkWindow     *window,
                             GdkEventFocus *focus_event,
                             gpointer       data G_GNUC_UNUSED)
{
	GvMainWindow *self = GV_MAIN_WINDOW(window);
	GvMainWindowPrivate *priv = self->priv;
	GvStationsTreeView *stations_tree_view = GV_STATIONS_TREE_VIEW(priv->stations_tree_view);

	g_assert(focus_event->type == GDK_FOCUS_CHANGE);

	//DEBUG("Main window %s focus", focus_event->in ? "gained" : "lost");

	if (focus_event->in)
		return FALSE;

	if (!POPUP_WINDOW_CLOSE_ON_FOCUS_OUT)
		return FALSE;

	if (gv_stations_tree_view_has_context_menu(stations_tree_view))
		return FALSE;

	gtk_window_close(window);

	return FALSE;
}

static gboolean
on_popup_window_key_press_event(GtkWindow   *window G_GNUC_UNUSED,
                                GdkEventKey *event,
                                gpointer     data G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	g_assert(event->type == GDK_KEY_PRESS);

	switch (event->keyval) {
	case GDK_KEY_Escape:
		/* Close window if 'Esc' key is pressed */
		gtk_window_close(window);
		break;

	case GDK_KEY_blank:
		/* Play/Stop when space is pressed */
		gv_player_toggle(player);
		break;

	default:
		break;
	}

	return FALSE;
}


/*
 * Standalone window signal handlers
 */

static gboolean
on_standalone_window_delete_event(GtkWindow *window,
                                  GdkEvent  *event G_GNUC_UNUSED,
                                  gpointer   data G_GNUC_UNUSED)
{
	/* Hide the window immediately */
	gtk_widget_hide(GTK_WIDGET(window));

	/* Now we're about to quit the application */
	gv_core_quit();

	/* We must return TRUE here to prevent the window from being destroyed.
	 * The window will be destroyed later on during the cleanup process.
	 */
	return TRUE;
}

static gboolean
on_standalone_window_key_press_event(GtkWindow   *window G_GNUC_UNUSED,
                                     GdkEventKey *event,
                                     gpointer     data G_GNUC_UNUSED)
{
	GvPlayer *player = gv_core_player;

	g_assert(event->type == GDK_KEY_PRESS);

	switch (event->keyval) {
	case GDK_KEY_Escape:
		/* Iconify window if 'Esc' key is pressed */
		gtk_window_iconify(window);
		break;

	case GDK_KEY_blank:
		/* Play/Stop when space is pressed */
		gv_player_toggle(player);
		break;

	default:
		break;
	}

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
setup_action(GtkWidget *widget, const gchar *widget_signal,
             GCallback callback, gpointer data)
{
	/* Signal handler */
	g_signal_connect(widget, widget_signal, callback, data);
}

static void
setup_setting(GtkWidget *widget, const gchar *widget_prop,
              GObject *obj, const gchar *obj_prop)
{
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
gv_main_window_populate_stations(GvMainWindow *self)
{
	GvMainWindowPrivate *priv = self->priv;
	GvStationsTreeView *tree_view = GV_STATIONS_TREE_VIEW(priv->stations_tree_view);

	gv_stations_tree_view_populate(tree_view);
}

void
gv_main_window_configure_for_popup(GvMainWindow *self)
{
	GtkApplicationWindow *application_window = GTK_APPLICATION_WINDOW(self);
	GtkWindow *window = GTK_WINDOW(self);

	/* Basically, we want the window to appear
	 * and behave as a popup window.
	 */

	/* Hide the menu bar in the main window */
	gtk_application_window_set_show_menubar(application_window, FALSE);

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

	/* Handle some keys */
	g_signal_connect(window, "key-press-event",
	                 G_CALLBACK(on_popup_window_key_press_event), NULL);

	/* Handle keyboard focus changes, so that we can hide the
	 * window on 'focus-out-event'.
	 */
	g_signal_connect(window, "focus-in-event",
	                 G_CALLBACK(on_popup_window_focus_change), NULL);
	g_signal_connect(window, "focus-out-event",
	                 G_CALLBACK(on_popup_window_focus_change), NULL);
}

void
gv_main_window_configure_for_standalone(GvMainWindow *self)
{
	GtkWindow *window = GTK_WINDOW(self);

	/* We want to quit the application when the window is closed */
	g_signal_connect(window, "delete-event",
	                 G_CALLBACK(on_standalone_window_delete_event), NULL);

	/* Handle some keys */
	g_signal_connect(window, "key-press-event",
	                 G_CALLBACK(on_standalone_window_key_press_event), NULL);
}

GtkWidget *
gv_main_window_new(GApplication *application)
{
	return g_object_new(GV_TYPE_MAIN_WINDOW,
	                    "application", application,
	                    NULL);
}

/*
 * Construct helpers
 */

static void
gv_main_window_populate_widgets(GvMainWindow *self)
{
	GvMainWindowPrivate *priv = self->priv;
	GtkBuilder *builder;
	gchar *uifile;

	/* Build the ui */
	gv_builder_load(UI_FILE, &builder, &uifile);
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
	priv->stations_tree_view = gv_stations_tree_view_new();
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
gv_main_window_setup_widgets(GvMainWindow *self)
{
	GvMainWindowPrivate *priv = self->priv;
	GObject *player_obj = G_OBJECT(gv_core_player);

	/* Setup adjustments - must be done first, before setting widget values */
	setup_adjustment(gtk_scale_button_get_adjustment(GTK_SCALE_BUTTON(priv->volume_button)),
	                 player_obj, "volume");

	/*
	 * Setup settings and actions.
	 * These function calls create the link between the widgets and the program.
	 * - settings: the link is a binding between the gtk widget and an internal object.
	 * - actions : the link is a callback invoked when the widget is clicked. This is a
	 * one-way link from the ui to the application, and additional work would be needed
	 * if the widget was to react to external changes.
	 */
	setup_action(priv->play_button, "clicked",
	             G_CALLBACK(on_button_clicked), self);

	setup_action(priv->prev_button, "clicked",
	             G_CALLBACK(on_button_clicked), self);

	setup_action(priv->next_button, "clicked",
	             G_CALLBACK(on_button_clicked), self);

	setup_setting(priv->repeat_toggle_button, "active",
	              player_obj, "repeat");

	setup_setting(priv->shuffle_toggle_button, "active",
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
gv_main_window_setup_layout(GvMainWindow *self)
{
	GvMainWindowPrivate *priv = self->priv;

	g_object_set(priv->window_vbox,
	             "spacing", GV_UI_ELEM_SPACING,
	             NULL);
}

/*
 * GObject methods
 */

static void
gv_main_window_finalize(GObject *object)
{
	GvMainWindow *self = GV_MAIN_WINDOW(object);
	GvPlayer *player = gv_core_player;

	TRACE("%p", object);

	/* Disconnect core signal handlers */
	g_signal_handlers_disconnect_by_data(player, self);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(gv_main_window, object);
}

static void
gv_main_window_constructed(GObject *object)
{
	GvMainWindow *self = GV_MAIN_WINDOW(object);
	GvPlayer *player = gv_core_player;

	/* Build window */
	gv_main_window_populate_widgets(self);
	gv_main_window_setup_widgets(self);
	gv_main_window_setup_layout(self);

	/* Connect core signal handlers */
	g_signal_connect(player, "notify",
	                 G_CALLBACK(on_player_notify), self);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(gv_main_window, object);
}

static void
gv_main_window_init(GvMainWindow *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = gv_main_window_get_instance_private(self);
}

static void
gv_main_window_class_init(GvMainWindowClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = gv_main_window_finalize;
	object_class->constructed = gv_main_window_constructed;
}
