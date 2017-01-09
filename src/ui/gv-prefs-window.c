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

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "additions/gtk.h"
#include "additions/glib-object.h"

#include "framework/log.h"
#include "framework/gv-framework.h"

#include "core/gv-core.h"

#include "ui/gv-ui.h"
#include "ui/gv-ui-helpers.h"
#include "ui/gv-ui-enum-types.h"
#include "ui/gv-tray.h"
#include "ui/gv-prefs-window.h"

#define UI_FILE "ui/prefs-window.glade"

/*
 * GObject definitions
 */

struct _GvPrefsWindowPrivate {
	/*
	 * Features
	 */

	/* Controls */
	GvFeature *hotkeys_feat;
	GvFeature *dbus_native_feat;
	GvFeature *dbus_mpris2_feat;
	/* Display */
	GvFeature *notifications_feat;
	GvFeature *console_output_feat;
	/* Player */
	GvFeature *inhibitor_feat;

	/*
	 * Widgets
	 */

	/* Top-level */
	GtkWidget *window_vbox;
	GtkWidget *notebook;
	/* Controls */
	GtkWidget *controls_vbox;
	GtkWidget *mouse_grid;
	GtkWidget *middle_click_action_label;
	GtkWidget *middle_click_action_combo;
	GtkWidget *scroll_action_label;
	GtkWidget *scroll_action_combo;
	GtkWidget *keyboard_grid;
	GtkWidget *hotkeys_label;
	GtkWidget *hotkeys_switch;
	GtkWidget *dbus_grid;
	GtkWidget *dbus_native_label;
	GtkWidget *dbus_native_switch;
	GtkWidget *dbus_mpris2_label;
	GtkWidget *dbus_mpris2_switch;
	/* Display */
	GtkWidget *display_vbox;
	GtkWidget *notif_vbox;
	GtkWidget *notif_enable_grid;
	GtkWidget *notif_enable_label;
	GtkWidget *notif_enable_switch;
	GtkWidget *notif_timeout_grid;
	GtkWidget *notif_timeout_check;
	GtkWidget *notif_timeout_spin;
	GObject   *notif_timeout_adj;
	GtkWidget *console_grid;
	GtkWidget *console_output_label;
	GtkWidget *console_output_switch;
	/* Player */
	GtkWidget *player_vbox;
	GtkWidget *player_grid;
	GtkWidget *autoplay_check;
	GtkWidget *inhibitor_label;
	GtkWidget *inhibitor_switch;
	/* Buttons */
	GtkWidget *close_button;
};

typedef struct _GvPrefsWindowPrivate GvPrefsWindowPrivate;

struct _GvPrefsWindow {
	/* Parent instance structure */
	GtkWindow              parent_instance;
	/* Private data */
	GvPrefsWindowPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(GvPrefsWindow, gv_prefs_window, GTK_TYPE_WINDOW)

/*
 * Gtk signal handlers
 */

static void
on_close_button_clicked(GtkButton *button G_GNUC_UNUSED, GvPrefsWindow *self)
{
	GtkWindow *window = GTK_WINDOW(self);

	gtk_window_close(window);
}

static gboolean
on_window_key_press_event(GvPrefsWindow *self, GdkEventKey *event, gpointer data G_GNUC_UNUSED)
{
	GtkWindow *window = GTK_WINDOW(self);

	g_assert(event->type == GDK_KEY_PRESS);

	if (event->keyval == GDK_KEY_Escape)
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

	/* Get property bounds, and assign it to the adjustment */
	g_object_get_property_uint_bounds(obj, obj_prop, &minimum, &maximum);
	gtk_adjustment_set_lower(adjustment, minimum);
	gtk_adjustment_set_upper(adjustment, maximum);
}

static void
setup_setting(const gchar *tooltip_text,
              GtkWidget *label, GtkWidget *widget, const gchar *widget_prop,
              GObject *obj, const gchar *obj_prop,
              GBindingTransformFunc transform_to,
              GBindingTransformFunc transform_from)
{
	/* Tooltip */
	if (tooltip_text) {
		gtk_widget_set_tooltip_text(widget, tooltip_text);
		if (label)
			gtk_widget_set_tooltip_text(label, tooltip_text);
	}

	/* Binding: obj 'prop' <-> widget 'prop'
	 * Order matters, don't mix up source and target here...
	 */
	g_object_bind_property_full(obj, obj_prop, widget, widget_prop,
	                            G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE,
	                            transform_to, transform_from, NULL, NULL);
}

static void
setup_feature(const gchar *tooltip_text, GtkWidget *label, GtkWidget *sw, GvFeature *feat)
{
	/* If feat is NULL, it's because it's been disabled at compile time */
	if (feat == NULL) {
		tooltip_text = _("Feature disabled at compile-time.");

		gtk_widget_set_tooltip_text(sw, tooltip_text);
		gtk_widget_set_tooltip_text(label, tooltip_text);
		gtk_widget_set_sensitive(label, FALSE);
		gtk_widget_set_sensitive(sw, FALSE);

		return;
	}

	/* Tooltip */
	if (tooltip_text) {
		gtk_widget_set_tooltip_text(sw, tooltip_text);
		gtk_widget_set_tooltip_text(label, tooltip_text);
	}

	/* Binding: feature 'enabled' <-> switch 'active'
	 * Order matters, don't mix up source and target here...
	 */
	g_object_bind_property(feat, "enabled", sw, "active",
	                       G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
}

static GvFeature *
find_feature(const gchar *type_name)
{
	GList *item;

	for (item = gv_framework_feature_list; item; item = item->next) {
		GObject *object;

		object = item->data;
		if (!g_strcmp0(type_name, G_OBJECT_TYPE_NAME(object)))
			return GV_FEATURE(object);
	}

	return NULL;
}

/*
 * Public methods
 */

GtkWidget *
gv_prefs_window_new(void)
{
	return g_object_new(GV_TYPE_PREFS_WINDOW, NULL);
}

/*
 * Construct helpers
 */

static void
gv_prefs_window_populate_features(GvPrefsWindow *self)
{
	GvPrefsWindowPrivate *priv = self->priv;

	/* Controls */
	priv->hotkeys_feat        = find_feature("GvHotkeys");
	priv->dbus_native_feat    = find_feature("GvDbusServerNative");
	priv->dbus_mpris2_feat    = find_feature("GvDbusServerMpris2");

	/* Display */
	priv->notifications_feat  = find_feature("GvNotifications");
	priv->console_output_feat = find_feature("GvConsoleOutput");

	/* Player */
	priv->inhibitor_feat      = find_feature("GvInhibitor");
}

static void
gv_prefs_window_populate_widgets(GvPrefsWindow *self)
{
	GvPrefsWindowPrivate *priv = self->priv;
	GtkBuilder *builder;
	gchar *uifile;

	/* Build the ui */
	gv_builder_load(UI_FILE, &builder, &uifile);
	DEBUG("Built from ui file '%s'", uifile);

	/* Save widget pointers */

	/* Top-level */
	GTK_BUILDER_SAVE_WIDGET(builder, priv, window_vbox);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, notebook);

	/* Controls */
	GTK_BUILDER_SAVE_WIDGET(builder, priv, controls_vbox);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, mouse_grid);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, middle_click_action_label);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, middle_click_action_combo);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, scroll_action_label);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, scroll_action_combo);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, keyboard_grid);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, hotkeys_label);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, hotkeys_switch);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, dbus_grid);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, dbus_native_label);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, dbus_native_switch);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, dbus_mpris2_label);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, dbus_mpris2_switch);

	/* Display */
	GTK_BUILDER_SAVE_WIDGET(builder, priv, display_vbox);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, notif_vbox);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, notif_enable_grid);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, notif_enable_label);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, notif_enable_switch);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, notif_timeout_grid);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, notif_timeout_check);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, notif_timeout_spin);
	GTK_BUILDER_SAVE_OBJECT(builder, priv, notif_timeout_adj);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, console_grid);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, console_output_label);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, console_output_switch);

	/* Player */
	GTK_BUILDER_SAVE_WIDGET(builder, priv, player_vbox);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, player_grid);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, autoplay_check);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, inhibitor_label);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, inhibitor_switch);

	/* Action area */
	GTK_BUILDER_SAVE_WIDGET(builder, priv, close_button);

	/* Pack that within the window */
	gtk_container_add(GTK_CONTAINER(self), priv->window_vbox);

	/* Cleanup */
	g_object_unref(G_OBJECT(builder));
	g_free(uifile);
}

static void
gv_prefs_window_setup_widgets(GvPrefsWindow *self)
{
	GvPrefsWindowPrivate *priv = self->priv;
	GObject *tray_obj   = G_OBJECT(gv_ui_tray);
	GObject *player_obj = G_OBJECT(gv_core_player);

	/* Setup adjustments
	 * This must be done before intializing any widget values.
	 */
	setup_adjustment(GTK_ADJUSTMENT(priv->notif_timeout_adj),
	                 G_OBJECT(priv->notifications_feat), "timeout-seconds");

	/* Setup conditionally sensitive widgets */
	g_object_bind_property(priv->notif_enable_switch, "active",
	                       priv->notif_timeout_grid, "sensitive",
	                       G_BINDING_SYNC_CREATE);
	g_object_bind_property(priv->notif_timeout_check, "active",
	                       priv->notif_timeout_spin, "sensitive",
	                       G_BINDING_SYNC_CREATE);

	/*
	 * Setup settings and features.
	 * These function calls create a binding between a gtk widget and
	 * an internal object, initializes the widget value, and set the
	 * widgets tooltips (label + setting).
	 */

	/* Controls */
	setup_setting(_("Action triggered by a middle click on the tray icon."),
	              priv->middle_click_action_label,
	              priv->middle_click_action_combo, "active-id",
	              tray_obj, "middle-click-action",
	              NULL, NULL);

	setup_setting(_("Action triggered by mouse-scrolling on the tray icon."),
	              priv->scroll_action_label,
	              priv->scroll_action_combo, "active-id",
	              tray_obj, "scroll-action",
	              NULL, NULL);

	setup_feature(_("Bind mutimedia keys (play/pause/stop/previous/next)."),
	              priv->hotkeys_label,
	              priv->hotkeys_switch,
	              priv->hotkeys_feat);

	setup_feature(_("Enable the native D-Bus server "
	                "(needed for the command-line interface)."),
	              priv->dbus_native_label,
	              priv->dbus_native_switch,
	              priv->dbus_native_feat);

	setup_feature(_("Enable the MPRIS2 D-Bus server."),
	              priv->dbus_mpris2_label,
	              priv->dbus_mpris2_switch,
	              priv->dbus_mpris2_feat);

	/* Display */
	setup_feature(_("Emit notifications when the status changes."),
	              priv->notif_enable_label,
	              priv->notif_enable_switch,
	              priv->notifications_feat);

	setup_setting(_("Whether to use a custom timeout for the notifications."),
	              NULL,
	              priv->notif_timeout_check, "active",
	              G_OBJECT(priv->notifications_feat), "timeout-enabled",
	              NULL, NULL);

	setup_setting(_("How long the notifications should be displayed (seconds)."),
	              NULL,
	              priv->notif_timeout_spin, "value",
	              G_OBJECT(priv->notifications_feat), "timeout-seconds",
	              NULL, NULL);

	setup_feature(_("Display information on the standard output."),
	              priv->console_output_label,
	              priv->console_output_switch,
	              priv->console_output_feat);

	/* Player */
	setup_setting(_("Whether to start playback automatically on startup."),
	              NULL,
	              priv->autoplay_check, "active",
	              player_obj, "autoplay",
	              NULL, NULL);

	setup_feature(_("Prevent the system from going to sleep while playing."),
	              priv->inhibitor_label,
	              priv->inhibitor_switch,
	              priv->inhibitor_feat);
}

static void
gv_prefs_window_setup_layout(GvPrefsWindow *self)
{
	GvPrefsWindowPrivate *priv = self->priv;

	/* Main */
	g_object_set(priv->window_vbox,
	             "margin", 0,
	             "spacing", 0,
	             NULL);

	/* Controls */
	g_object_set(priv->controls_vbox,
	             "margin", GV_UI_WINDOW_BORDER,
	             "spacing", GV_UI_GROUP_SPACING,
	             NULL);
	g_object_set(priv->mouse_grid,
	             "row-spacing", GV_UI_ELEM_SPACING,
	             "column-spacing", GV_UI_LABEL_SPACING,
	             "halign", GTK_ALIGN_END,
	             NULL);
	g_object_set(priv->keyboard_grid,
	             "row-spacing", GV_UI_ELEM_SPACING,
	             "column-spacing", GV_UI_LABEL_SPACING,
	             "halign", GTK_ALIGN_END,
	             NULL);
	g_object_set(priv->dbus_grid,
	             "row-spacing", GV_UI_ELEM_SPACING,
	             "column-spacing", GV_UI_LABEL_SPACING,
	             "halign", GTK_ALIGN_END,
	             NULL);

	/* Display */
	g_object_set(priv->display_vbox,
	             "margin", GV_UI_WINDOW_BORDER,
	             "spacing", GV_UI_GROUP_SPACING,
	             NULL);

	g_object_set(priv->notif_enable_grid,
	             "row-spacing", GV_UI_ELEM_SPACING,
	             "column-spacing", GV_UI_LABEL_SPACING,
	             "margin-bottom", GV_UI_ELEM_SPACING,
	             NULL);
	g_object_set(priv->notif_timeout_grid,
	             "halign", GTK_ALIGN_END,
	             "row-spacing", GV_UI_ELEM_SPACING,
	             "column-spacing", GV_UI_LABEL_SPACING,
	             NULL);

	g_object_set(priv->console_grid,
	             "row-spacing", GV_UI_ELEM_SPACING,
	             "column-spacing", GV_UI_LABEL_SPACING,
	             "halign", GTK_ALIGN_END,
	             NULL);

	/* Player */
	g_object_set(priv->player_vbox,
	             "margin", GV_UI_WINDOW_BORDER,
	             "spacing", GV_UI_GROUP_SPACING,
	             NULL);
	g_object_set(priv->player_grid,
	             "row-spacing", GV_UI_ELEM_SPACING,
	             "column-spacing", GV_UI_LABEL_SPACING,
	             "halign", GTK_ALIGN_END,
	             NULL);

}

/*
 * GObject methods
 */

static void
gv_prefs_window_finalize(GObject *object)
{
	TRACE("%p", object);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(gv_prefs_window, object);
}

static void
gv_prefs_window_constructed(GObject *object)
{
	GvPrefsWindow *self = GV_PREFS_WINDOW(object);
	GvPrefsWindowPrivate *priv = self->priv;
	GtkWindow *window = GTK_WINDOW(object);

	/* Build the window */
	gv_prefs_window_populate_features(self);
	gv_prefs_window_populate_widgets(self);
	gv_prefs_window_setup_widgets(self);
	gv_prefs_window_setup_layout(self);

	/* Configure the window behavior */
	gtk_window_set_title(window, PACKAGE_CAMEL_NAME " Preferences");
	gtk_window_set_skip_taskbar_hint(window, TRUE);
	gtk_window_set_resizable(window, FALSE);
	gtk_window_set_modal(window, TRUE);

	/* Connect signal handlers */
	g_signal_connect(priv->close_button, "clicked",
	                 G_CALLBACK(on_close_button_clicked), self);
	g_signal_connect(self, "key_press_event",
	                 G_CALLBACK(on_window_key_press_event), NULL);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(gv_prefs_window, object);
}

static void
gv_prefs_window_init(GvPrefsWindow *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = gv_prefs_window_get_instance_private(self);
}

static void
gv_prefs_window_class_init(GvPrefsWindowClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = gv_prefs_window_finalize;
	object_class->constructed = gv_prefs_window_constructed;
}
