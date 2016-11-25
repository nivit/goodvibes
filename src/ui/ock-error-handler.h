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

#ifndef __OVERCOOKED_UI_OCK_ERROR_HANDLER_H__
#define __OVERCOOKED_UI_OCK_ERROR_HANDLER_H__

#include <glib-object.h>

/* GObject declarations */

#define OCK_TYPE_ERROR_HANDLER ock_error_handler_get_type()

G_DECLARE_FINAL_TYPE(OckErrorHandler, ock_error_handler, OCK, ERROR_HANDLER, GObject)

/* Methods */

OckErrorHandler *ock_error_handler_new(void);

void ock_error_handler_watch  (OckErrorHandler *self);
void ock_error_handler_unwatch(OckErrorHandler *self);

#endif /* __OVERCOOKED_UI_OCK_ERROR_HANDLER_H__ */
