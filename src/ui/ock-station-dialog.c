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

#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "additions/gtk.h"
#include "additions/glib-object.h"

#include "framework/log.h"
#include "framework/ock-framework.h"

#include "core/ock-core.h"

#include "ui/ock-builder-helpers.h"
#include "ui/global.h"
#include "ui/ock-station-dialog.h"

#define UI_FILE "ui/station-dialog.glade"

/*
 * GObject definitions
 */

struct _OckStationDialogPrivate {
	/* Widgets */
	/* Top-level */
	GtkWidget *main_grid;
	/* Entries */
	GtkWidget *name_entry;
	GtkWidget *uri_entry;
	/* Buttons */
	GtkWidget *save_button;
};

typedef struct _OckStationDialogPrivate OckStationDialogPrivate;

struct _OckStationDialog {
	/* Parent instance structure */
	GtkDialog                parent_instance;
	/* Private data */
	OckStationDialogPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(OckStationDialog, ock_station_dialog, GTK_TYPE_DIALOG)

/*
 * Helpers
 */

static void
g_str_remove_weird_chars(const gchar *text, gchar **out, guint *out_len)
{
	gchar *start, *ptr;
	guint length, i;

	length = strlen(text);
	start = g_malloc(length + 1);

	for (i = 0, ptr = start; i < length; i++) {
		/* Discard every character below space */
		if (text[i] > ' ')
			*ptr++ = text[i];
	}

	*ptr = '\0';

	if (length != ptr - start) {
		length = ptr - start;
		start = g_realloc(start, length);
	}

	*out = start;
	*out_len = length;
}

/*
 * Signal handlers
 */

static void
on_uri_entry_insert_text(GtkEditable *editable,
                         gchar       *text,
                         gint         length G_GNUC_UNUSED,
                         gpointer     position,
                         OckStationDialog *self)
{
	gchar *new_text;
	guint new_len;

	/* Remove weird characters */
	g_str_remove_weird_chars(text, &new_text, &new_len);

	/* Replace text in entry */
	g_signal_handlers_block_by_func(editable,
	                                on_uri_entry_insert_text,
	                                self);

	gtk_editable_insert_text(editable, new_text, new_len, position);

	g_signal_handlers_unblock_by_func(editable,
	                                  on_uri_entry_insert_text,
	                                  self);

	/* We inserted the text in the entry, so now we just need to
	 * prevent the default handler to run.
	 */
	g_signal_stop_emission_by_name(editable, "insert-text");

	/* Free */
	g_free(new_text);
}

static void
on_uri_entry_changed(GtkEditable *editable,
                     OckStationDialog *self)
{
	OckStationDialogPrivate *priv = self->priv;
	guint text_len;

	text_len = gtk_entry_get_text_length(GTK_ENTRY(editable));

	/* If the entry is empty, the save button is not clickable */
	if (text_len > 0)
		gtk_widget_set_sensitive(priv->save_button, TRUE);
	else
		gtk_widget_set_sensitive(priv->save_button, FALSE);
}

/*
 * Private methods
 */

static void
ock_station_dialog_build(OckStationDialog *self)
{
	OckStationDialogPrivate *priv = self->priv;
	GtkWidget *content_area;
	GtkBuilder *builder;
	gchar *uifile;

	/* Build the ui */
	ock_builder_load(UI_FILE, &builder, &uifile);
	DEBUG("Built from ui file '%s'", uifile);

	/* Save widget pointers */

	/* Top-level */
	GTK_BUILDER_SAVE_WIDGET(builder, priv, main_grid);

	/* Text entries */
	GTK_BUILDER_SAVE_WIDGET(builder, priv, name_entry);
	GTK_BUILDER_SAVE_WIDGET(builder, priv, uri_entry);

	/* Configure uri widgets */
	gtk_entry_set_input_purpose(GTK_ENTRY(priv->uri_entry), GTK_INPUT_PURPOSE_URL);
	g_signal_connect(priv->uri_entry, "insert-text",
	                 G_CALLBACK(on_uri_entry_insert_text),
	                 self);
	g_signal_connect(priv->uri_entry, "changed",
	                 G_CALLBACK(on_uri_entry_changed),
	                 self);

	/* Configure the content area */
	content_area = gtk_dialog_get_content_area(GTK_DIALOG(self));
	gtk_container_add(GTK_CONTAINER(content_area), priv->main_grid);
	gtk_widget_set_margins(content_area, 6);
	gtk_box_set_spacing(GTK_BOX(content_area), 6);

	/* Configure the action area.
	 * We don't allow creating a station without an empty URI.
	 * Therefore, the save button is clickable only when the URI empty
	 * contains some text.
	 */
	gtk_dialog_add_buttons(GTK_DIALOG(self),
	                       "Cancel", GTK_RESPONSE_CANCEL,
	                       "Save", GTK_RESPONSE_OK,
	                       NULL);
	priv->save_button = gtk_dialog_get_widget_for_response
	                    (GTK_DIALOG(self), GTK_RESPONSE_OK);
	gtk_widget_set_sensitive(priv->save_button, FALSE);

	/* Cleanup */
	g_object_unref(G_OBJECT(builder));
	g_free(uifile);
}

/*
 * Public methods
 */

void
ock_station_dialog_populate(OckStationDialog *self, OckStation *station)
{
	OckStationDialogPrivate *priv = self->priv;
	const gchar *station_name, *station_uri;
	gchar *window_title;

	station_name = ock_station_get_name(station);
	station_uri = ock_station_get_uri(station);

	/* Set windows title */
	window_title = g_strdup("Edit station");
	if (station_name)
		window_title = g_strdup_printf("%s '%s'", window_title, station_name);
	gtk_window_set_title(GTK_WINDOW(self), window_title);
	g_free(window_title);

	/* Populate entries */
	if (station_name)
		gtk_entry_set_text(GTK_ENTRY(priv->name_entry), station_name);
	if (station_uri)
		gtk_entry_set_text(GTK_ENTRY(priv->uri_entry), station_uri);
}

void
ock_station_dialog_retrieve(OckStationDialog *self, OckStation *station)
{
	OckStationDialogPrivate *priv = self->priv;
	const gchar *name, *uri;

	name = gtk_entry_get_text(GTK_ENTRY(priv->name_entry));
	uri = gtk_entry_get_text(GTK_ENTRY(priv->uri_entry));

	g_object_set(station,
	             "name", name,
	             "uri", uri,
	             NULL);
}

OckStation *
ock_station_dialog_retrieve_new(OckStationDialog *self)
{
	OckStationDialogPrivate *priv = self->priv;
	const gchar *name, *uri;
	OckStation *station;

	name = gtk_entry_get_text(GTK_ENTRY(priv->name_entry));
	uri = gtk_entry_get_text(GTK_ENTRY(priv->uri_entry));

	station = ock_station_new(name, uri);
	return station;
}

GtkWidget *
ock_station_dialog_new(void)
{
	return g_object_new(OCK_TYPE_STATION_DIALOG, NULL);
}

/*
 * GObject methods
 */

static void
ock_station_dialog_constructed(GObject *object)
{
	OckStationDialog *self = OCK_STATION_DIALOG(object);
	GtkWindow *window = GTK_WINDOW(object);

	/* Build window */
	ock_station_dialog_build(self);

	/* Configure window */
	gtk_window_set_skip_taskbar_hint(window, TRUE);
	gtk_window_set_modal(window, TRUE);
	gtk_window_set_default_size(window, 480, -1);
	gtk_window_set_title(window, "Add new station");

	/* Set transient parent */
	gtk_window_set_transient_for(window, GTK_WINDOW(ock_ui_main_window));

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_station_dialog, object);
}

static void
ock_station_dialog_init(OckStationDialog *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_station_dialog_get_instance_private(self);
}

static void
ock_station_dialog_class_init(OckStationDialogClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->constructed = ock_station_dialog_constructed;
}
