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

#include "framework/log.h"

#include "core/ock-core.h"

#include "ui/ock-stations-tree-view.h"
#include "ui/ock-station-context-menu.h"

/*
 * GObject definitions
 */

struct _OckStationsTreeViewPrivate {
	/* Dragging operation in progress */
	gboolean is_dragging;
	OckStation *station_dragged;
	gint station_new_pos;
};

typedef struct _OckStationsTreeViewPrivate OckStationsTreeViewPrivate;

struct _OckStationsTreeView {
	/* Parent instance structure */
	GtkTreeView parent_instance;
	/* Private data */
	OckStationsTreeViewPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(OckStationsTreeView, ock_stations_tree_view, GTK_TYPE_TREE_VIEW)

/*
 * Columns
 */

enum {
	STATION_COLUMN,
	STATION_NAME_COLUMN,
	STATION_WEIGHT_COLUMN,
	STATION_STYLE_COLUMN,
	N_COLUMNS
};

/*
 * Player signal handlers
 */

static void
on_player_notify_station(OckPlayer           *player,
                         GParamSpec          *pspec G_GNUC_UNUSED,
                         OckStationsTreeView *self)
{
	GtkTreeView *tree_view = GTK_TREE_VIEW(self);
	GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);
	OckStation *station = ock_player_get_station(player);
	GtkTreeIter iter;
	gboolean can_iter;

	DEBUG("Notify from player");

	can_iter = gtk_tree_model_get_iter_first(tree_model, &iter);

	while (can_iter) {
		OckStation *iter_station;

		/* Get station from model */
		gtk_tree_model_get(tree_model, &iter,
		                   STATION_COLUMN, &iter_station,
		                   -1);

		/* Make the current station bold */
		if (station == iter_station)
			gtk_list_store_set(GTK_LIST_STORE(tree_model), &iter,
			                   STATION_WEIGHT_COLUMN, PANGO_WEIGHT_BOLD,
			                   -1);
		else
			gtk_list_store_set(GTK_LIST_STORE(tree_model), &iter,
			                   STATION_WEIGHT_COLUMN, PANGO_WEIGHT_NORMAL,
			                   -1);

		/* Unref the station */
		g_object_unref(iter_station);

		/* Next ! */
		can_iter = gtk_tree_model_iter_next(tree_model, &iter);
	}
}

/*
 * Station List signal handlers
 * Needed to update the internal list store when the station list is modified.
 * (remember the station list might be updated through the D-Bus API).
 */

static void
on_station_list_station_event(OckStationList *station_list G_GNUC_UNUSED,
                              OckStation     *station G_GNUC_UNUSED,
                              OckStationsTreeView  *self)
{
	ock_stations_tree_view_populate(self);
}

static GSignalHandler station_list_handlers[] = {
	{ "station-added",    G_CALLBACK(on_station_list_station_event) },
	{ "station-removed",  G_CALLBACK(on_station_list_station_event) },
	{ "station-modified", G_CALLBACK(on_station_list_station_event) },
	{ "station-moved",    G_CALLBACK(on_station_list_station_event) },
	{ NULL,               NULL                                      }
};

#if 0
/*
 * Stations tree view row activated
 * Might be caused by mouse action (single click on the row),
 * or by keyboard action (Enter or similar key pressed).
 */

static void
on_tree_view_row_activated(OckStationsTreeView *self,
                           GtkTreePath         *path,
                           GtkTreeViewColumn   *column G_GNUC_UNUSED,
                           gpointer             data G_GNUC_UNUSED)
{
	OckStationsTreeViewPrivate *priv = self->priv;
	GtkTreeView *tree_view = GTK_TREE_VIEW(self);
	GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);
	GtkTreeIter iter;
	OckStation *station;

	/* Get station from model */
	gtk_tree_model_get_iter(tree_model, &iter, path);
	gtk_tree_model_get(tree_model, &iter,
	                   STATION_COLUMN, &station,
	                   -1);

	/* Play station */
	if (station) {
		OckPlayer *player = ock_core_player;

		ock_player_set_station(player, station);
		ock_player_play(player);
		g_object_unref(station);
	}

	DEBUG("Row activated");
}
#else
static gboolean
idle_tree_view_row_activated(OckStationsTreeView *self)
{
	OckStationsTreeViewPrivate *priv = self->priv;
	GtkTreeView *tree_view = GTK_TREE_VIEW(self);
	GtkTreeSelection *tree_selection = gtk_tree_view_get_selection(tree_view);
	GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);
	OckPlayer *player = ock_core_player;
	GtkTreeIter iter;
	OckStation *station;

	/* Check if a drag operation is in progress */
	if (priv->is_dragging) {
		DEBUG("Drag'n'drop operation in progress");
		return FALSE;
	}

	/* Get station */
	gtk_tree_selection_get_selected(tree_selection, &tree_model, &iter);
	gtk_tree_model_get(tree_model, &iter,
	                   STATION_COLUMN, &station,
	                   -1);

	/* Station might be NULL if the station list is empty */
	if (station == NULL)
		return FALSE;

	/* Play station */
	ock_player_set_station(player, station);
	ock_player_play(player);

	/* Cleanup */
	g_object_unref(station);

	return FALSE;
}

/*
 * Stations Tree View row-activated
 * Might be caused by mouse action (single click on the row),
 * or by keyboard action (Enter or similar key pressed).
 */

static void
on_tree_view_row_activated(OckStationsTreeView *self,
                           GtkTreePath         *path G_GNUC_UNUSED,
                           GtkTreeViewColumn   *column G_GNUC_UNUSED,
                           gpointer             data G_GNUC_UNUSED)
{
	DEBUG("Row activated, delaying...");

	/* This signal might be received when the user clicks an item,
	 * when he hits 'Enter' or similar on the keyboards,
	 * but also when a drag'n'drop operation is performed, BEFORE
	 * the 'drag-begin' signal is send.
	 * In such case, we want to do nothing, but we don't know yet
	 * that a drag'n'drop was started...
	 * So, we have to delay the execution of the code a little bit.
	 * We must give time to the 'drag-begin' signal to be emitted.
	 */

	g_idle_add((GSourceFunc) idle_tree_view_row_activated, self);
}
#endif

/*
 * Stations Tree View button-press-event, for context menu on right-click
 */

static void
on_context_menu_hide(GtkWidget *widget, gpointer data G_GNUC_UNUSED)
{
	/* Context menu can be destroyed */
	g_object_unref(widget);
}

static gboolean
on_tree_view_button_press_event(OckStationsTreeView *self,
                                GdkEventButton      *event,
                                gpointer             data G_GNUC_UNUSED)

{
	GtkTreeView *tree_view = GTK_TREE_VIEW(self);
	GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);
	GtkTreePath *path;
	GtkTreeIter iter;

	DEBUG("Button pressed: %d", event->button);

	/* Ensure the event happened on the expected window.
	 * According from the doc, we MUST check that.
	 */
	if (event->window != gtk_tree_view_get_bin_window(tree_view))
		return FALSE;

	/* Handle only single-click */
	if (event->type != GDK_BUTTON_PRESS)
		return FALSE;

	/* Handle only right-click */
	if (event->button != 3)
		return FALSE;

	/* Get row at this position */
	path = NULL;
	gtk_tree_view_get_path_at_pos(tree_view, event->x, event->y,
	                              &path, NULL, NULL, NULL);

	if (path == NULL)
		return FALSE;

	/* Get corresponding station */
	OckStation *station;
	gtk_tree_model_get_iter(tree_model, &iter, path);
	gtk_tree_model_get(tree_model, &iter,
	                   STATION_COLUMN, &station,
	                   -1);

	/* Create the context menu */
	GtkWidget *context_menu;
	if (station) {
		context_menu = ock_station_context_menu_new_with_station(station);
		g_object_unref(station);
	} else {
		context_menu = ock_station_context_menu_new();
	}

	/* We're the owner, therefore responsible for finalization */
	g_object_ref_sink(context_menu);
	g_signal_connect(context_menu, "hide", G_CALLBACK(on_context_menu_hide), NULL);

#if GTK_CHECK_VERSION(3,22,0)
	gtk_menu_popup_at_pointer(GTK_MENU(context_menu), NULL);
#else
	gtk_menu_popup(GTK_MENU(context_menu),
	               NULL,
	               NULL,
	               NULL,
	               NULL,
	               event->button,
	               event->time);
#endif

	/* Free at last */
	gtk_tree_path_free(path);

	return FALSE;
}

/*
 * Stations Tree View drag-and-drop source signal handlers.
 * We watch these signals to know when a dragging operation is in progress.
 */

static void
on_tree_view_drag_begin(OckStationsTreeView *self,
                        GdkDragContext      *context G_GNUC_UNUSED,
                        gpointer             data G_GNUC_UNUSED)
{
	OckStationsTreeViewPrivate *priv = self->priv;

	priv->is_dragging = TRUE;
}

static void
on_tree_view_drag_end(OckStationsTreeView *self,
                      GdkDragContext      *context G_GNUC_UNUSED,
                      gpointer             data G_GNUC_UNUSED)
{
	OckStationsTreeViewPrivate *priv = self->priv;

	priv->is_dragging = FALSE;
}


static gboolean
on_tree_view_drag_failed(OckStationsTreeView *self,
                         GdkDragContext      *context G_GNUC_UNUSED,
                         GtkDragResult        result,
                         gpointer             data G_GNUC_UNUSED)
{
	OckStationsTreeViewPrivate *priv = self->priv;

	DEBUG("Drag failed with result: %d", result);
	priv->is_dragging = FALSE;

	return TRUE;
}

static GSignalHandler tree_view_drag_handlers[] = {
	{ "drag-begin",  G_CALLBACK(on_tree_view_drag_begin)  },
	{ "drag-end",    G_CALLBACK(on_tree_view_drag_end)    },
	{ "drag-failed", G_CALLBACK(on_tree_view_drag_failed) },
	{ NULL,          NULL                                 }
};

/*
 * Stations List Store signal handlers.
 * We watch these signals to be notified when a station is moved
 * in the list (this is done when user drag'n'drop).
 * After a station has been moved, we need to forward this change
 * to the core station list.
 *
 * The signal sequence when a station is moved is as follow:
 * - row-inserted: a new empty row is created
 * - row-changed: the new row has been populated
 * - row-deleted: the old row has been deleted
 */

static void
on_list_store_row_inserted(GtkTreeModel        *tree_model G_GNUC_UNUSED,
                           GtkTreePath         *path,
                           GtkTreeIter         *iter G_GNUC_UNUSED,
                           OckStationsTreeView *self)
{
	OckStationsTreeViewPrivate *priv = self->priv;
	gint *indices;
	guint position;

	/* This should only happen when there's a drag-n-drop */
	if (priv->is_dragging == FALSE) {
		WARNING("Not dragging at the moment, ignoring");
		return;
	}

	/* We expect a clean status */
	if (priv->station_dragged != NULL || priv->station_new_pos != -1) {
		WARNING("Current state is not clean, ignoring");
		return;
	}

	/* Get position of the new row */
	indices = gtk_tree_path_get_indices(path);
	position = indices[0];
	priv->station_new_pos = position;

	DEBUG("Row inserted at %d", position);
}

static void
on_list_store_row_changed(GtkTreeModel        *tree_model,
                          GtkTreePath         *path,
                          GtkTreeIter         *iter,
                          OckStationsTreeView *self)
{
	OckStationsTreeViewPrivate *priv = self->priv;
	gint *indices;
	gint position;

	/* We only care if it's caused by a drag'n'drop */
	if (priv->is_dragging == FALSE)
		return;

	/* We expect the indice to be the one registered previously
	 * in 'row-inserted' signal.
	 */
	indices = gtk_tree_path_get_indices(path);
	position = indices[0];
	if (position != priv->station_new_pos) {
		WARNING("Unexpected position %d, doesn't match %d",
		        position, priv->station_new_pos);
		return;
	}

	/* Get station */
	OckStation *station;
	gtk_tree_model_get(tree_model, iter,
	                   STATION_COLUMN, &station,
	                   -1);

	/* Save it */
	priv->station_dragged = station;

	/* Freedom for the braves */
	g_object_unref(station);

	DEBUG("Row changed at %d", position);
}

static void
on_list_store_row_deleted(GtkTreeModel        *tree_model G_GNUC_UNUSED,
                          GtkTreePath         *path,
                          OckStationsTreeView *self)
{
	OckStationsTreeViewPrivate *priv = self->priv;
	gint *indices;
	guint indice_deleted, indice_inserted;

	/* End of drag operation, let's commit that to station list */
	OckStation *station = priv->station_dragged;
	if (station == NULL) {
		WARNING("Station dragged is null, wtf ?");
		return;
	}

	/* Compute indice */
	indices = gtk_tree_path_get_indices(path);
	indice_deleted = indices[0];
	indice_inserted = priv->station_new_pos;
	if (indice_deleted < indice_inserted)
		indice_inserted -= 1;

	/* Move station in the station list */
	OckStationList *station_list = ock_core_station_list;

	g_signal_handlers_block(station_list, station_list_handlers, self);
	ock_station_list_move(station_list, station, indice_inserted);
	g_signal_handlers_unblock(station_list, station_list_handlers, self);

	DEBUG("Row deleted, moving sta at %d", indice_inserted);

	/* Clean status */
	priv->station_dragged = NULL;
	priv->station_new_pos = -1;
}

static GSignalHandler list_store_handlers[] = {
	{ "row-inserted",          G_CALLBACK(on_list_store_row_inserted) },
	{ "row-changed",           G_CALLBACK(on_list_store_row_changed)  },
	{ "row-deleted",           G_CALLBACK(on_list_store_row_deleted)  },
	{ NULL,                    NULL                        }
};

/*
 * Helpers
 */

static void
station_cell_data_func(GtkTreeViewColumn *tree_column G_GNUC_UNUSED,
                       GtkCellRenderer   *cell,
                       GtkTreeModel      *tree_model,
                       GtkTreeIter       *iter,
                       gpointer           data G_GNUC_UNUSED)
{
	gchar *station_name;
	PangoWeight station_weight;
	PangoStyle station_style;

	/* According to the doc, there should be nothing heavy in this function,
	 * since it's called intensively. No UTF-8 conversion, for example.
	 */

	gtk_tree_model_get(tree_model, iter,
	                   STATION_NAME_COLUMN, &station_name,
	                   STATION_WEIGHT_COLUMN, &station_weight,
	                   STATION_STYLE_COLUMN, &station_style,
	                   -1);

	g_object_set(cell,
	             "text", station_name,
	             "weight", station_weight,
	             "style", station_style,
	             NULL);

	g_free(station_name);
}

/*
 * Public methods
 */

void
ock_stations_tree_view_populate(OckStationsTreeView *self)
{
	GtkTreeView *tree_view = GTK_TREE_VIEW(self);
	GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);
	GtkListStore *list_store = GTK_LIST_STORE(tree_model);

	OckStationList *station_list = ock_core_station_list;
	OckPlayer *player = ock_core_player;

	TRACE("%p", self);

	/* Block list store handlers */
	g_signal_handlers_block(list_store, list_store_handlers, self);

	/* Make station list empty */
	gtk_list_store_clear(list_store);

	/* Handle the special-case: empty station list */
	if (ock_station_list_get_length(station_list) == 0) {
		GtkTreeIter tree_iter;

		/* Populate */
		gtk_list_store_append(list_store, &tree_iter);
		gtk_list_store_set(list_store, &tree_iter,
		                   STATION_COLUMN, NULL,
		                   STATION_NAME_COLUMN, "Right click to add station",
		                   STATION_WEIGHT_COLUMN, PANGO_WEIGHT_NORMAL,
		                   STATION_STYLE_COLUMN, PANGO_STYLE_ITALIC,
		                   -1);

		/* Configure behavior */
		gtk_tree_view_set_hover_selection(tree_view, FALSE);
		gtk_tree_view_set_activate_on_single_click(tree_view, FALSE);

	} else {
		OckStation *current_station = ock_player_get_station(player);
		OckStation *station;
		OckStationListIter *iter;

		/* Populate menu with every station */
		iter = ock_station_list_iter_new(station_list);

		while (ock_station_list_iter_loop(iter, &station)) {
			GtkTreeIter tree_iter;
			const gchar *station_name;
			PangoWeight weight;

			station_name = ock_station_get_name_or_uri(station);

			if (station == current_station)
				weight = PANGO_WEIGHT_BOLD;
			else
				weight = PANGO_WEIGHT_NORMAL;

			gtk_list_store_append(list_store, &tree_iter);
			gtk_list_store_set(list_store, &tree_iter,
			                   STATION_COLUMN, station,
			                   STATION_NAME_COLUMN, station_name,
			                   STATION_WEIGHT_COLUMN, weight,
			                   STATION_STYLE_COLUMN, PANGO_STYLE_NORMAL,
			                   -1);
		}
		ock_station_list_iter_free(iter);

		/* Configure behavior */
		gtk_tree_view_set_hover_selection(tree_view, TRUE);
		gtk_tree_view_set_activate_on_single_click(tree_view, TRUE);
	}

	/* Unblock list store handlers */
	g_signal_handlers_unblock(list_store, list_store_handlers, self);
}

GtkWidget *
ock_stations_tree_view_new(void)
{
	return g_object_new(OCK_TYPE_STATIONS_TREE_VIEW, NULL);
}

/*
 * GObject methods
 */

static void
ock_stations_tree_view_constructed(GObject *object)
{
	GtkTreeView *tree_view = GTK_TREE_VIEW(object);
	OckStationsTreeView *self = OCK_STATIONS_TREE_VIEW(object);

	/* Hide headers */
	gtk_tree_view_set_headers_visible(tree_view, FALSE);

	/* Enable hover selection mode, and single click activation */
	gtk_tree_view_set_hover_selection(tree_view, TRUE);
	gtk_tree_view_set_activate_on_single_click(tree_view, TRUE);

	/* Allow re-ordering. The tree view then becomes a drag source and
	 * a drag destination, and all the drag-and-drop mess is handled
	 * by the tree view. We will just have to watch the drag-* signals.
	 */
	gtk_tree_view_set_reorderable(tree_view, TRUE);

	/* Horizontally, the tree view grows as wide as the longer station name.
	 * I'm OK with this behavior, let it be.
	 */

	/* Vertically, the tree view grows forever. If someone has too many stations,
	 * it might cause a problem.
	 * I tried to put the tree view inside a scrolled window, but it creates more
	 * problems than it solves, mainly because then we have to assign a fixed size
	 * to the tree view. Finding the appropriate size (that would be the natural,
	 * expanded size if NOT within a scrolled window, OR the screen height if too
	 * many stations) seems VERY VERY tricky and slippery...
	 */

	/*
	 * Create the stations list store. It has 4 columns:
	 * - the station object
	 * - the station represented by a string (for displaying)
	 * - the station's font weight (bold characters for current station)
	 * - the station's font style (italic characters if no station)
	 */

	/* Create a new list store */
	GtkListStore *list_store;
	list_store = gtk_list_store_new(4, G_TYPE_OBJECT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT);

	/* Associate it with the tree view */
	gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(list_store));
	g_object_unref(list_store);

	/*
	 * Create the column that will be displayed
	 */

	/* Create a renderer */
	GtkCellRenderer *renderer;
	renderer = gtk_cell_renderer_text_new();

	/* Create a column that uses this renderer */
	GtkTreeViewColumn *column;
	column = gtk_tree_view_column_new_with_attributes
	         ("Station", renderer, NULL);

	/* Set the function that will render this column */
	gtk_tree_view_column_set_cell_data_func(column, renderer,
	                                        station_cell_data_func,
	                                        NULL, NULL);

	/* Append the column */
	gtk_tree_view_append_column(tree_view, column);

	/*
	 * Tree View signal handlers
	 */

	/* Left click or keyboard */
	g_signal_connect(tree_view, "row-activated",
	                 G_CALLBACK(on_tree_view_row_activated), NULL);

	/* We handle the right-click here */
	g_signal_connect(tree_view, "button-press-event",
	                 G_CALLBACK(on_tree_view_button_press_event), NULL);

	/* Drag-n-drop signal handlers.
	 * We need to watch it just to know when a drag-n-drop is in progress.
	 */
	g_signal_handlers_connect(tree_view, tree_view_drag_handlers, NULL);

	/*
	 * List Store signal handlers
	 */

	/* If stations are re-ordered, we have to propagate to the core */
	g_signal_handlers_connect(list_store, list_store_handlers, self);

	/*
	 * Core signal handlers
	 */

	OckPlayer *player = ock_core_player;
	OckStationList *station_list = ock_core_station_list;

	g_signal_connect(player, "notify::station",
	                 G_CALLBACK(on_player_notify_station), self);
	g_signal_handlers_connect(station_list, station_list_handlers, self);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_stations_tree_view, object);
}

static void
ock_stations_tree_view_init(OckStationsTreeView *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_stations_tree_view_get_instance_private(self);

	/* Initialize internal state */
	self->priv->station_new_pos = -1;
}

static void
ock_stations_tree_view_class_init(OckStationsTreeViewClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->constructed = ock_stations_tree_view_constructed;
}
