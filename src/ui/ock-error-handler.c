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

#include "additions/glib-object.h"

#include "framework/log.h"
#include "framework/ock-framework.h"

#include "ui/ock-error-handler.h"

// TODO Finish to implement the whole thing

/*
 * GObject definitions
 */

struct _OckErrorHandler {
	/* Parent instance structure */
	GObject parent_instance;
};

G_DEFINE_TYPE(OckErrorHandler, ock_error_handler, G_TYPE_OBJECT)

/*
 * Helpers
 */

/*
 * Signal handlers & callbacks
 */

static void
on_errorable_error(GObject *obj, const gchar *string, OckErrorHandler *self)
{
	WARNING("Got string: %s", string);

	// TODO: popup error message

	(void) obj;
	(void) self;
}

/*
 * Public methods
 */

void
ock_error_handler_unwatch(OckErrorHandler *self)
{
	GList *item;

	for (item = ock_framework_errorable_list; item; item = item->next) {
		GObject *object;

		object = item->data;
		g_signal_handlers_disconnect_by_data(object, self);
	}
}

void
ock_error_handler_watch(OckErrorHandler *self)
{
	GList *item;

	for (item = ock_framework_errorable_list; item; item = item->next) {
		GObject *object;

		object = item->data;
		g_signal_connect(object, "error", G_CALLBACK(on_errorable_error), self);
	}
}

OckErrorHandler *
ock_error_handler_new(void)
{
	return g_object_new(OCK_TYPE_ERROR_HANDLER, NULL);
}

/*
 * GObject methods
 */

static void
ock_error_handler_finalize(GObject *object)
{
	TRACE("%p", object);

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_error_handler, object);
}

static void
ock_error_handler_constructed(GObject *object)
{
	TRACE("%p", object);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_error_handler, object);
}

static void
ock_error_handler_init(OckErrorHandler *self)
{
	TRACE("%p", self);
}

static void
ock_error_handler_class_init(OckErrorHandlerClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_error_handler_finalize;
	object_class->constructed = ock_error_handler_constructed;
}
