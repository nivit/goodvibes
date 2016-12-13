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

#include "additions/gtk.h"
#include "additions/glib-object.h"

#include "framework/log.h"
#include "framework/ock-framework.h"

#include "ui/global.h"
#include "ui/ock-about-dialog.h"
#include "ui/ock-prefs-window.h"
#include "ui/ock-main-menu.h"

#define PREFS_LABEL _("Preferences")
#define ABOUT_LABEL _("About")
#define QUIT_LABEL  _("Quit")

/*
 * GObject definitions
 */

struct _OckMainMenuPrivate {
	GtkWidget *prefs_window;
};

typedef struct _OckMainMenuPrivate OckMainMenuPrivate;

struct _OckMainMenu {
	/* Parent instance structure */
	GtkMenu             parent_instance;
	/* Private data */
	OckMainMenuPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(OckMainMenu, ock_main_menu, GTK_TYPE_MENU)

/*
 * Signal handlers
 */

static void
on_prefs_window_destroy(GtkWidget *window, OckMainMenu *self)
{
	OckMainMenuPrivate *priv = self->priv;

	g_assert(window == priv->prefs_window);
	priv->prefs_window = NULL;
}

/*
 * Gtk signal handlers
 */

static void
on_menu_item_activate(GtkMenuItem *item, OckMainMenu *self)
{
	OckMainMenuPrivate *priv = self->priv;
	const gchar *label;

	/* Get item label */
	label = gtk_menu_item_get_label(item);

	if (label == NULL) {
		CRITICAL("Failed to get label from menu item");

	} else if (!g_strcmp0(label, PREFS_LABEL)) {
		GtkWidget *window;

		window = priv->prefs_window;
		if (window == NULL) {
			window = ock_prefs_window_new();
			g_signal_connect(window, "destroy",
			                 G_CALLBACK(on_prefs_window_destroy), self);
			priv->prefs_window = window;
		}

		gtk_window_present(GTK_WINDOW(window));

	} else if (!g_strcmp0(label, ABOUT_LABEL)) {
		ock_show_about_dialog(GTK_WINDOW(ock_ui_main_window));

	} else if (!g_strcmp0(label, QUIT_LABEL)) {
		ock_framework_quit_loop();

	} else {
		CRITICAL("Unhandled label '%s'", label);
	}
}

/*
 * Private methods
 */

static void
ock_main_menu_populate(OckMainMenu *self)
{
	GtkWidget *widget;

	/* Preferences */
	widget = gtk_menu_item_new_with_label(PREFS_LABEL);
	gtk_menu_shell_append(GTK_MENU_SHELL(self), widget);
	g_signal_connect(widget, "activate", G_CALLBACK(on_menu_item_activate), self);

	/* Separator */
	widget = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(self), widget);

	/* About */
	widget = gtk_menu_item_new_with_label(ABOUT_LABEL);
	gtk_menu_shell_append(GTK_MENU_SHELL(self), widget);
	g_signal_connect(widget, "activate", G_CALLBACK(on_menu_item_activate), self);

	/* Quit */
	widget = gtk_menu_item_new_with_label(QUIT_LABEL);
	gtk_menu_shell_append(GTK_MENU_SHELL(self), widget);
	g_signal_connect(widget, "activate", G_CALLBACK(on_menu_item_activate), self);

	/* Showtime */
	gtk_widget_show_all(GTK_WIDGET(self));
}

/*
 * Public methods
 */

GtkWidget *
ock_main_menu_new(void)
{
	return g_object_new(OCK_TYPE_MAIN_MENU, NULL);
}

/*
 * GObject methods
 */

static void
ock_main_menu_finalize(GObject *object)
{
	OckMainMenu *self = OCK_MAIN_MENU(object);
	OckMainMenuPrivate *priv = self->priv;

	TRACE("%p", object);

	/* Free resources */
	if (priv->prefs_window) {
		/* gtk_window_close() seems to be async, and using it here has no effect.
		 * So we use gtk_widget_destroy() instead, which is the right thing to do
		 * according to the documentation:
		 *
		 * https://developer.gnome.org/gtk3/stable/GtkWindow.html#gtk-window-new
		 */
		gtk_widget_destroy(GTK_WIDGET(priv->prefs_window));
	}

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_main_menu, object);
}

static void
ock_main_menu_constructed(GObject *object)
{
	OckMainMenu *self = OCK_MAIN_MENU(object);

	TRACE("%p", object);

	/* Populate */
	ock_main_menu_populate(self);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_main_menu, object);
}

static void
ock_main_menu_init(OckMainMenu *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_main_menu_get_instance_private(self);
}

static void
ock_main_menu_class_init(OckMainMenuClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_main_menu_finalize;
	object_class->constructed = ock_main_menu_constructed;
}
