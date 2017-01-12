/*
 * Goodvibes Radio Player
 *
 * Copyright (C) 2015-2017 Arnaud Rebillout
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

#ifndef __GOODVIBES_FRAMEWORK_GV_FILE_HELPERS_H__
#define __GOODVIBES_FRAMEWORK_GV_FILE_HELPERS_H__

#include <glib.h>

/* File I/O */

gboolean gv_file_read_sync (const gchar *path, gchar **text, GError **err);
gboolean gv_file_write_sync(const gchar *path, const gchar *text, GError **err);

/* Pathes */

const gchar        *gv_get_current_config_dir(void);
const gchar        *gv_get_current_data_dir  (void);
const gchar        *gv_get_user_config_dir   (void);
const gchar        *gv_get_user_data_dir     (void);
const gchar *const *gv_get_system_config_dirs(void);
const gchar *const *gv_get_system_data_dirs  (void);

typedef enum {
	GV_DIR_CURRENT_CONFIG = (1 << 0),
	GV_DIR_CURRENT_DATA   = (1 << 1),
	GV_DIR_USER_CONFIG    = (1 << 2),
	GV_DIR_USER_DATA      = (1 << 3),
	GV_DIR_SYSTEM_CONFIG  = (1 << 4),
	GV_DIR_SYSTEM_DATA    = (1 << 5),
} GvDirType;

GSList *gv_get_path_list(GvDirType dir_type, const gchar *filename);
GSList *gv_get_existing_path_list(GvDirType dir_type, const gchar *filename);
gchar  *gv_get_first_existing_path(GvDirType dir_type, const gchar *filename);

#endif /* __GOODVIBES_FRAMEWORK_GV_FILE_HELPERS_H__ */
