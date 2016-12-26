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

#ifndef __GOODVIBES_OPTIONS_H__
#define __GOODVIBES_OPTIONS_H__

struct options {
	/* Options */
	gboolean     background;
	gboolean     colorless;
	const gchar *log_level;
	const gchar *output_file;
	gboolean     print_version;
	gboolean     without_ui;
	/* Arguments */
	const gchar *uri_to_play;
};

extern struct options options;

void options_parse(int *argc, char **argv[]);
void options_cleanup(void);

#endif /* __GOODVIBES_OPTIONS_H__ */
