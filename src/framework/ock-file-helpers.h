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

#ifndef __OVERCOOKED_FRAMEWORK_OCK_FILE_HELPERS_H__
#define __OVERCOOKED_FRAMEWORK_OCK_FILE_HELPERS_H__

#include <glib.h>

/* File I/O */

gboolean ock_file_read_sync (const gchar *path, gchar **text, GError **err);
gboolean ock_file_write_sync(const gchar *path, const gchar *text, GError **err);

/* Pathes */

const gchar        *ock_get_current_config_dir(void);
const gchar        *ock_get_current_data_dir  (void);
const gchar        *ock_get_user_config_dir   (void);
const gchar        *ock_get_user_data_dir     (void);
const gchar *const *ock_get_system_config_dirs(void);
const gchar *const *ock_get_system_data_dirs  (void);

typedef enum {
	OCK_DIR_CURRENT_CONFIG = (1 << 0),
	OCK_DIR_CURRENT_DATA   = (1 << 1),
	OCK_DIR_USER_CONFIG    = (1 << 2),
	OCK_DIR_USER_DATA      = (1 << 2),
	OCK_DIR_SYSTEM_CONFIG  = (1 << 2),
	OCK_DIR_SYSTEM_DATA    = (1 << 2),
} OckDirType;

GSList *ock_get_path_list(OckDirType dir_type, const gchar *filename);
GSList *ock_get_existing_path_list(OckDirType dir_type, const gchar *filename);
gchar  *ock_get_first_existing_path(OckDirType dir_type, const gchar *filename);

#endif /* __OVERCOOKED_FRAMEWORK_OCK_FILE_HELPERS_H__ */
