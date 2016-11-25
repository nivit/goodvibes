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

#ifndef __OVERCOOKED_FRAMEWORK_OCK_ERRORABLE_H__
#define __OVERCOOKED_FRAMEWORK_OCK_ERRORABLE_H__

#include <glib-object.h>

/* GObject declarations */

#define OCK_TYPE_ERRORABLE ock_errorable_get_type()

G_DECLARE_INTERFACE(OckErrorable, ock_errorable, OCK, ERRORABLE, GObject)

struct _OckErrorableInterface {
	/* Parent interface */
	GTypeInterface parent_iface;

	/* Signals */
	void (*error) (OckErrorable *self);
};

/* Methods */

void ock_errorable_emit_error(OckErrorable *self, const gchar *string);
void ock_errorable_emit_error_printf(OckErrorable *self, const gchar *fmt, ...);

#endif /* __OVERCOOKED_FRAMEWORK_OCK_ERRORABLE_H__ */
