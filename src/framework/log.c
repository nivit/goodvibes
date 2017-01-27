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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <glib-object.h>

#include "framework/vt-codes.h"

/* Additional log level for traces */

#define LOG_LEVEL_TRACE    G_LOG_LEVEL_DEBUG << 1

/* Error printing */

#define perrorf(fmt, ...) fprintf(stderr, fmt ": %s\n", ##__VA_ARGS__, strerror(errno))
#define print_err(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

/* Predefined log strings */

struct _log_strings {
	/* Log level prefixes */
	const gchar *error;
	const gchar *critical;
	const gchar *warning;
	const gchar *message;
	const gchar *info;
	const gchar *debug;
	const gchar *trace;
	const gchar *dfl;
	/* Colors codes */
	const gchar *reset;
	const gchar *dim;
};

typedef struct _log_strings LogStrings;

static LogStrings log_strings_colorless = {
	/* Log level prefixes */
	.error    = "ERR ",
	.critical = "CRIT",
	.warning  = "WARN",
	.message  = "MSG ",
	.info     = "INFO",
	.debug    = "DBG ",
	.trace    = " -> ",
	.dfl      = "LOG ",
	/* Colors codes */
	.reset    = "",
	.dim      = ""
};

static LogStrings log_strings_colorful = {
	/* Log level prefixes */
	.error    = VT_RED   ("ERR "),
	.critical = VT_RED   ("CRIT"),
	.warning  = VT_YELLOW("WARN"),
	.message  = VT_GREEN ("MSG "),
	.info     = VT_GREEN ("INFO"),
	.debug    = VT_DIM   ("DBG "),
	.trace    = VT_DIM   (" -> "),
	.dfl      =           "LOG ",
	/* Color codes */
	.reset    = VT_CODE_ESC VT_CODE_RESET,
	.dim      = VT_CODE_ESC VT_CODE_DIM
};

/* Global variables that control the behavior of logs */

static FILE *log_stream;
static gint log_level;
static LogStrings *log_strings = &log_strings_colorless;

/* Copies of std{out/err}, in case we redirect it */

static int stdout_copy = -1;
static int stderr_copy = -1;

/* Convert from string to log level */
static gint
string_to_log_level(const gchar *str)
{
	GLogLevelFlags level = G_LOG_LEVEL_MESSAGE;

	if (str == NULL)
		return level;

	if (!strcasecmp(str, "error") ||
	    !strcasecmp(str, "err")) {
		level = G_LOG_LEVEL_ERROR;
	} else if (!strcasecmp(str, "critical") ||
	           !strcasecmp(str, "crit")) {
		level = G_LOG_LEVEL_CRITICAL;
	} else if (!strcasecmp(str, "warning") ||
	           !strcasecmp(str, "warn")) {
		level = G_LOG_LEVEL_WARNING;
	} else if (!strcasecmp(str, "message") ||
	           !strcasecmp(str, "msg")) {
		level = G_LOG_LEVEL_MESSAGE;
	} else if (!strcasecmp(str, "info")) {
		level = G_LOG_LEVEL_INFO;
	} else if (!strcasecmp(str, "debug") ||
	           !strcasecmp(str, "dbg")) {
		level = G_LOG_LEVEL_DEBUG;
	} else if (!strcasecmp(str, "trace")) {
		level = LOG_LEVEL_TRACE;
	} else {
		print_err("Invalid log level '%s'", str);
	}

	return level;
}

/* Default log handler.
 * We DON'T honor any environment variables, such as
 * G_MESSAGES_PREFIXED, G_MESSAGES_DEBUG, ...
 */
static void
log_default_handler(const gchar   *domain,
                    GLogLevelFlags level,
                    const gchar   *msg,
                    gpointer       unused_data G_GNUC_UNUSED)
{
	GDateTime *now;
	gchar *now_str;
	const gchar *prefix;

	level &= G_LOG_LEVEL_MASK;

	/* Last chance to discard the log */
	if (level > log_level)
		return;

	/* Discard debug messages that don't belong to us */
	if (domain && level > G_LOG_LEVEL_INFO)
		return;

	/* Start by getting the current time. Note that it would be more
	 * accurate to get the time earlier, but we don't need such accuracy
	 * I believe, plus it's more convenient to do it here.
	 */
	now = g_date_time_new_now_local();
	now_str = g_date_time_format(now, "%T");

	/* Prefix depends on log level.
	 * Cast is needed at the moment to avoid a gcc warning.
	 * This is because GLogLevelFlags *may be* 8-bits long
	 * due to the way it's defined.
	 * Check the net for more info:
	 * https://mail.gnome.org/archives/gtk-devel-list/2014-May/msg00029.html
	 * https://bugzilla.gnome.org/show_bug.cgi?id=730932
	 */
	switch ((gint) level) {
	case G_LOG_LEVEL_ERROR:
		prefix = log_strings->error;
		break;
	case G_LOG_LEVEL_CRITICAL:
		prefix = log_strings->critical;
		break;
	case G_LOG_LEVEL_WARNING:
		prefix = log_strings->warning;
		break;
	case G_LOG_LEVEL_MESSAGE:
		prefix = log_strings->message;
		break;
	case G_LOG_LEVEL_INFO:
		prefix = log_strings->info;
		break;
	case G_LOG_LEVEL_DEBUG:
		prefix = log_strings->debug;
		break;
	case LOG_LEVEL_TRACE:
		prefix = log_strings->trace;
		break;
	default:
		prefix = log_strings->dfl;
		break;
	}

	/* Send everything to the log stream */
	fputs(prefix, log_stream);
	fputs(" ", log_stream);

	fputs(log_strings->dim, log_stream);
	fputs(now_str, log_stream);
	fputs(log_strings->reset, log_stream);
	fputs(" ", log_stream);

	if (domain)
		fprintf(log_stream, "[%s] ", domain);

	fputs(msg, log_stream);

	fputs("\n", log_stream);

	/* Cleanup */
	g_date_time_unref(now);
	g_free(now_str);
}

void
log_trace_property_access(const gchar *file, const gchar *func, GObject *object,
                          guint property_id, const GValue *value, GParamSpec *pspec,
                          gboolean print_value)
{
	gchar *value_string;
	guint max_len = 128;

	if (LOG_LEVEL_TRACE > log_level)
		return;

	if (print_value) {
		value_string = g_strdup_value_contents(value);
		if (value_string && strlen(value_string) > max_len) {
			guint idx;

			if (value_string[0] == '"')
				idx = max_len - 4;
			else
				idx = max_len - 3;

			value_string[idx++] = '.';
			value_string[idx++] = '.';
			value_string[idx++] = '.';

			if (value_string[0] == '"')
				value_string[idx++] = '"';

			value_string[idx] = '\0';
		}
	} else {
		value_string = g_strdup_printf("(%s)", G_VALUE_TYPE_NAME(value));
	}

	g_log(G_LOG_DOMAIN, LOG_LEVEL_TRACE, "%s%s: %s%s(%p, %d, %s, '%s')",
	      log_strings->dim, file, func, log_strings->reset,
	      object, property_id, value_string, pspec->name);

	g_free(value_string);
}

void
log_trace(const gchar *file, const gchar *func, const gchar *fmt, ...)
{
	va_list ap;
	gchar fmt2[512];

	if (LOG_LEVEL_TRACE > log_level)
		return;

	snprintf(fmt2, sizeof fmt2, "%s%s: %s()%s: (%s)",
	         log_strings->dim, file, func, log_strings->reset, fmt);

	va_start(ap, fmt);
	g_logv(G_LOG_DOMAIN, LOG_LEVEL_TRACE, fmt2, ap);
	va_end(ap);
}

void
log_msg(GLogLevelFlags level, const gchar *file, const gchar *func, const gchar *fmt, ...)
{
	va_list ap;
	gchar fmt2[512];

	if (level > log_level)
		return;

	if (!file && !func)
		snprintf(fmt2, sizeof fmt2, "%s", fmt);
	else
		snprintf(fmt2, sizeof fmt2, "%s%s: %s()%s: %s",
		         log_strings->dim, file, func, log_strings->reset, fmt);

	va_start(ap, fmt);
	g_logv(G_LOG_DOMAIN, level, fmt2, ap);
	va_end(ap);
}

void
log_cleanup(void)
{
	/* Restore standard output */
	if (stdout_copy > 0) {
		if (dup2(stdout_copy, STDOUT_FILENO) == -1)
			perror("Failed to restore stdout");
		if (close(stdout_copy) == -1)
			perror("Failed to close stdout copy");
	}

	/* Restore error output */
	if (stderr_copy > 0) {
		if (dup2(stderr_copy, STDERR_FILENO) == -1)
			perror("Failed to restore stderr");
		if (close(stderr_copy) == -1)
			perror("Failed to close stderr copy");
	}
}

void
log_init(const gchar *log_level_str,
         gboolean     colorless,
         const gchar *output_file)
{
	/* We send every log message to stderr, so that it's easy
	 * to separate logs (intended for developpers) and messages
	 * (intended for users).
	 */
	log_stream = stderr;

	/* We have our own log handler */
	g_log_set_default_handler(log_default_handler, NULL);

	/* Set log level */
	log_level = string_to_log_level(log_level_str);

	/* Redirect output to a log file */
	if (output_file) {
		FILE *fp;

		stdout_copy = dup(STDOUT_FILENO);
		if (stdout_copy == -1)
			perror("Failed to duplicate stdout");

		stderr_copy = dup(STDERR_FILENO);
		if (stderr_copy == -1)
			perror("Failed to duplicate stderr");

		fp = fopen(output_file, "w");
		if (fp) {
			if (dup2(fileno(fp), STDOUT_FILENO) == -1)
				perror("Failed to redirect stdout");
			if (dup2(fileno(fp), STDERR_FILENO) == -1)
				perror("Failed to redirect stderr");
			if (fclose(fp) == -1)
				perrorf("Failed to close log file '%s'", output_file);
		} else
			perrorf("Failed to open log file '%s'", output_file);
	}

	/* Set colorful log prefixes.
	 * Colors only make sense if logs are sent to a terminal,
	 * since they're implemented with VT commands.
	 */
	if (isatty(fileno(log_stream)) && !colorless)
		log_strings = &log_strings_colorful;
}
