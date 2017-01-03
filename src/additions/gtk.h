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

#ifndef __GOODVIBES_ADDITIONS_GTK_H__
#define __GOODVIBES_ADDITIONS_GTK_H__

#include <gtk/gtk.h>

/*
 * Version Information
 */

const gchar *gtk_get_runtime_version_string(void);
const gchar *gtk_get_compile_version_string(void);

/*
 * GtkBuilder
 */

#define gtk_builder_get_widget(builder, name)     \
	GTK_WIDGET(gtk_builder_get_object(builder, name))

#define gtk_builder_get_object(builder, name)     \
	G_OBJECT(gtk_builder_get_object(builder, name))

/*
 * Convenient macros to save widgets from a GtkBuilder instance.
 * If the widget is not found, it is considered as a programming error,
 * therefore ending the execution of the program.
 * This macro also ensures that the widget name in the ui file is the same
 * as the one used in the C file, therefore enforcing a consistent naming
 * across the code.
 */

#define GTK_BUILDER_SAVE_WIDGET(builder, str, name)       \
	do { \
		str->name = gtk_builder_get_widget(builder, #name); \
		g_assert(GTK_IS_WIDGET(str->name)); \
	} while (0)

#define GTK_BUILDER_SAVE_OBJECT(builder, str, name)       \
	do { \
		str->name = gtk_builder_get_object(builder, #name); \
		g_assert(G_IS_OBJECT(str->name)); \
	} while (0)

/*
 * GtkWidget
 */

#define gtk_widget_set_margins(widget, margin)  \
	do { \
		gtk_widget_set_margin_start(widget, margin); \
		gtk_widget_set_margin_end(widget, margin); \
		gtk_widget_set_margin_top(widget, margin); \
		gtk_widget_set_margin_bottom(widget, margin); \
	} while (0)

#endif /* __GOODVIBES_ADDITIONS_GTK_H__ */
