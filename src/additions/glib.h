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

#ifndef __OVERCOOKED_ADDITIONS_GLIB_H__
#define __OVERCOOKED_ADDITIONS_GLIB_H__

#include <glib.h>

/*
 * Version Information
 */

const gchar *glib_get_runtime_version_string(void);
const gchar *glib_get_compile_version_string(void);

/*
 * String Utilities Functions
 */

gchar *g_strjoin_null(const gchar *separator, unsigned int n_strings, ...);

/*
 * GVariant
 */

#define g_variant_builder_add_dictentry_string(b, key, val)             \
	g_variant_builder_add(b, "{sv}", key, g_variant_new_string(val))

#define g_variant_builder_add_dictentry_object_path(b, key, val)        \
	g_variant_builder_add(b, "{sv}", key, g_variant_new_object_path(val))

void g_variant_builder_add_dictentry_array_string(GVariantBuilder *b,
                                                  const gchar     *key,
                                                  ...) G_GNUC_NULL_TERMINATED;

#endif /* __OVERCOOKED_ADDITIONS_GLIB_H__ */
