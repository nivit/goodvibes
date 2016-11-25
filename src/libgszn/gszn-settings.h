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

#ifndef __OVERCOOKED_LIBGSZN_GSZN_SETTINGS_H__
#define __OVERCOOKED_LIBGSZN_GSZN_SETTINGS_H__

#include <glib-object.h>

/* GObject declarations */

#define GSZN_TYPE_SETTINGS gszn_settings_get_type()

GType gszn_settings_get_type(void) G_GNUC_CONST;

/* Data types */

typedef gchar *(*GsznTweakString)(const gchar *);

typedef struct {
	GType           backend_type;
	GsznTweakString ser_object_name;
	GsznTweakString deser_object_name;
	GsznTweakString ser_property_name;
	GsznTweakString deser_property_name;
} GsznSettings;

/* Functions */

GsznSettings *gszn_settings_new (void);
void          gszn_settings_free(GsznSettings *settings);

#endif /* __OVERCOOKED_LIBGSZN_GSZN_SETTINGS_H__ */
