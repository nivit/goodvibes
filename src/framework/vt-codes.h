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

#ifndef __GOODVIBES_FRAMEWORK_VT_CODES_H__
#define __GOODVIBES_FRAMEWORK_VT_CODES_H__

/* Terminal codes (ANSI/VT100) */

#define VT_CODE_ESC "\033"

#define VT_CODE_RESET  "[0m"
#define VT_CODE_BOLD   "[1m"
#define VT_CODE_DIM    "[2m"

#define VT_CODE_BLACK  "[0;30m"
#define VT_CODE_RED    "[0;31m"
#define VT_CODE_GREEN  "[0;32m"
#define VT_CODE_BROWN  "[0;33m"
#define VT_CODE_BLUE   "[0;34m"
#define VT_CODE_PURPLE "[0;35m"
#define VT_CODE_CYAN   "[0;36m"
#define VT_CODE_GREY   "[0;37m"

#define VT_CODE_DARK_GREY    "[1;30m"
#define VT_CODE_LIGHT_RED    "[1;31m"
#define VT_CODE_LIGHT_GREEN  "[1;32m"
#define VT_CODE_YELLOW       "[1;33m"
#define VT_CODE_LIGHT_BLUE   "[1;34m"
#define VT_CODE_LIGHT_PURPLE "[1;35m"
#define VT_CODE_LIGHT_CYAN   "[1;36m"
#define VT_CODE_WHITE        "[1;37m"

/* Convenient macros to wrap static strings */

#define VT_BOLD(str) VT_CODE_ESC VT_CODE_BOLD str VT_CODE_ESC VT_CODE_RESET
#define VT_DIM(str)  VT_CODE_ESC VT_CODE_DIM  str VT_CODE_ESC VT_CODE_RESET

#define VT_BLACK(str)  VT_CODE_ESC VT_CODE_BLACK  str VT_CODE_ESC VT_CODE_RESET
#define VT_RED(str)    VT_CODE_ESC VT_CODE_RED    str VT_CODE_ESC VT_CODE_RESET
#define VT_GREEN(str)  VT_CODE_ESC VT_CODE_GREEN  str VT_CODE_ESC VT_CODE_RESET
#define VT_BROWN(str)  VT_CODE_ESC VT_CODE_BROWN  str VT_CODE_ESC VT_CODE_RESET
#define VT_BLUE(str)   VT_CODE_ESC VT_CODE_BLUE   str VT_CODE_ESC VT_CODE_RESET
#define VT_PURPLE(str) VT_CODE_ESC VT_CODE_PURPLE str VT_CODE_ESC VT_CODE_RESET
#define VT_CYAN(str)   VT_CODE_ESC VT_CODE_CYAN   str VT_CODE_ESC VT_CODE_RESET
#define VT_GREY(str)   VT_CODE_ESC VT_CODE_GREY   str VT_CODE_ESC VT_CODE_RESET

#define VT_DARK_GREY(str)    VT_CODE_ESC VT_CODE_DARK_GREY    str VT_CODE_ESC VT_CODE_RESET
#define VT_LIGHT_RED(str)    VT_CODE_ESC VT_CODE_LIGHT_RED    str VT_CODE_ESC VT_CODE_RESET
#define VT_LIGHT_GREEN(str)  VT_CODE_ESC VT_CODE_LIGHT_GREEN  str VT_CODE_ESC VT_CODE_RESET
#define VT_YELLOW(str)       VT_CODE_ESC VT_CODE_YELLOW       str VT_CODE_ESC VT_CODE_RESET
#define VT_LIGHT_BLUE(str)   VT_CODE_ESC VT_CODE_LIGHT_BLUE   str VT_CODE_ESC VT_CODE_RESET
#define VT_LIGHT_PURPLE(str) VT_CODE_ESC VT_CODE_LIGHT_PURPLE str VT_CODE_ESC VT_CODE_RESET
#define VT_LIGHT_CYAN(str)   VT_CODE_ESC VT_CODE_LIGHT_CYAN   str VT_CODE_ESC VT_CODE_RESET
#define VT_WHITE(str)        VT_CODE_ESC VT_CODE_WHITE        str VT_CODE_ESC VT_CODE_RESET

#endif /* __GOODVIBES_FRAMEWORK_VT_CODES_H__ */
