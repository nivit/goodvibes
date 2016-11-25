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

#include <glib-object.h>

#include "libgszn/gszn-settings.h"

static GsznSettings *
gszn_settings_copy(GsznSettings *settings)
{
	return g_memdup(settings, sizeof(GsznSettings));
}

void
gszn_settings_free(GsznSettings *settings)
{
	g_free(settings);
}

GsznSettings *
gszn_settings_new(void)
{
	return g_new0(GsznSettings, 1);
}

G_DEFINE_BOXED_TYPE(GsznSettings, gszn_settings, gszn_settings_copy, gszn_settings_free);
