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

#include "ui/global.h"
#include "ui/gv-about-dialog.h"
#include "ui/gv-prefs-window.h"
#include "ui/gv-main-menu.h"

#define PREFS_LABEL _("Preferences")
#define ABOUT_LABEL _("About")
#define QUIT_LABEL  _("Quit")

/*
 * GObject definitions
 */

struct _GvMainMenuPrivate {
	/* Widgets */
	GtkWidget *prefs_menu_item;
	GtkWidget *about_menu_item;
	GtkWidget *quit_menu_item;
	/* Currently opened preferences window */
	GtkWidget *prefs_window;
};

typedef struct _GvMainMenuPrivate GvMainMenuPrivate;

struct _GvMainMenu {
	/* Parent instance structure */
	GtkMenu             parent_instance;
	/* Private data */
	GvMainMenuPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(GvMainMenu, gv_main_menu, GTK_TYPE_MENU)

/*
 * Signal handlers
 */

static void
on_prefs_window_destroy(GtkWidget *window, GvMainMenu *self)
{
	GvMainMenuPrivate *priv = self->priv;

	g_assert(window == priv->prefs_window);
	priv->prefs_window = NULL;
}

/*
 * Gtk signal handlers
 */

static void
on_menu_item_activate(GtkMenuItem *item, GvMainMenu *self)
{
	GvMainMenuPrivate *priv = self->priv;
	GtkWidget *widget = GTK_WIDGET(item);

	if (widget == priv->prefs_menu_item) {
		GtkWidget *window;

		window = priv->prefs_window;
		if (window == NULL) {
			window = gv_prefs_window_new();
			g_signal_connect(window, "destroy",
			                 G_CALLBACK(on_prefs_window_destroy), self);
			priv->prefs_window = window;
		}

		gtk_window_present(GTK_WINDOW(window));

	} else if (widget == priv->about_menu_item) {
		gv_show_about_dialog(GTK_WINDOW(gv_ui_main_window));

	} else if (widget == priv->quit_menu_item) {
		gv_framework_quit_loop();

	} else {
		CRITICAL("Unhandled menu item %p", item);
	}
}

/*
 * Private methods
 */

static void
gv_main_menu_populate(GvMainMenu *self)
{
	GvMainMenuPrivate *priv = self->priv;
	GtkWidget *widget;

	/* This is supposed to be called once only */
	g_assert(gtk_container_get_children(GTK_CONTAINER(self)) == NULL);

	/* Preferences */
	widget = gtk_menu_item_new_with_label(PREFS_LABEL);
	gtk_menu_shell_append(GTK_MENU_SHELL(self), widget);
	g_signal_connect(widget, "activate", G_CALLBACK(on_menu_item_activate), self);
	priv->prefs_menu_item = widget;

	/* Separator */
	widget = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(self), widget);

	/* About */
	widget = gtk_menu_item_new_with_label(ABOUT_LABEL);
	gtk_menu_shell_append(GTK_MENU_SHELL(self), widget);
	g_signal_connect(widget, "activate", G_CALLBACK(on_menu_item_activate), self);
	priv->about_menu_item = widget;

	/* Quit */
	widget = gtk_menu_item_new_with_label(QUIT_LABEL);
	gtk_menu_shell_append(GTK_MENU_SHELL(self), widget);
	g_signal_connect(widget, "activate", G_CALLBACK(on_menu_item_activate), self);
	priv->quit_menu_item = widget;

	/* Showtime */
	gtk_widget_show_all(GTK_WIDGET(self));
}

/*
 * Public methods
 */

GtkWidget *
gv_main_menu_new(void)
{
	return g_object_new(GV_TYPE_MAIN_MENU, NULL);
}

/*
 * GObject methods
 */

static void
gv_main_menu_finalize(GObject *object)
{
	GvMainMenu *self = GV_MAIN_MENU(object);
	GvMainMenuPrivate *priv = self->priv;

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
	G_OBJECT_CHAINUP_FINALIZE(gv_main_menu, object);
}

static void
gv_main_menu_constructed(GObject *object)
{
	GvMainMenu *self = GV_MAIN_MENU(object);

	TRACE("%p", object);

	/* Populate */
	gv_main_menu_populate(self);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(gv_main_menu, object);
}

static void
gv_main_menu_init(GvMainMenu *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = gv_main_menu_get_instance_private(self);
}

static void
gv_main_menu_class_init(GvMainMenuClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = gv_main_menu_finalize;
	object_class->constructed = gv_main_menu_constructed;
}
