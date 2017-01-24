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
#include <string.h>
#include <ctype.h>

#include <glib.h>
#include <gio/gio.h>

/* http://misc.flogisoft.com/bash/tip_colors_and_formatting */
#define ESC        "\033"
#define BOLD_CODE  "[1m"
#define RESET_CODE "[0m"
#define BOLD(str)  ESC BOLD_CODE str ESC RESET_CODE

#define print(fmt, ...)     fprintf(stdout, fmt"\n", ##__VA_ARGS__)
#define print_err(fmt, ...) fprintf(stderr, fmt"\n", ##__VA_ARGS__)

/*
 * Help
 */

const char *app_name;

void
help_init(const char *str)
{
	app_name = str;
}

void
help_and_exit(int exit_code)
{

#define REVISION(name)     print("%s (version " PACKAGE_VERSION ")", name);
#define USAGE(name)        print("Usage: %s <command> [<args>]", name);
#define TITLE(str)         print(BOLD(str ":"))
#define COMMAND(cmd, desc) print(BOLD("  %-32s") "%s", cmd, desc)
#define DESC(desc)         print("  %-32s%s", "", desc)
#define NL()               print("");

	REVISION(app_name);
	USAGE(app_name);
	NL();

	TITLE  ("Base commands");
	COMMAND("launch", "Launch " PACKAGE_CAMEL_NAME);
	COMMAND("quit", "Quit " PACKAGE_CAMEL_NAME);
	COMMAND("help", "Print this help message");
	NL();

	TITLE  ("Control");
	print  (". <station> can be the station name or uri");
	COMMAND("play [<station>]", "Without argument, play the current station");
	DESC   ("Otherwise, play the station given in argument");
	COMMAND("stop", "Stop playback");
	COMMAND("next", "Play next station");
	COMMAND("prev(ious)", "Play previous station");
	COMMAND("volume  [<value>]", "Get/set volume (in %)");
	COMMAND("mute    [true/false]", "Get/set mute state");
	COMMAND("repeat  [true/false]", "Get/set repeat");
	COMMAND("shuffle [true/false]", "Get/set shuffle");
	COMMAND("current", "Get info on current station");
	COMMAND("playing", "Get playback status");
	NL();

	TITLE  ("Station list");
	print  (". <station> can be the station name or uri");
	COMMAND("list", "Display the list of stations");
	COMMAND("add    <station-uri> [<station-name>] [[first/last] [before/after <station>]]", "");
	DESC   ("Add a station to the list");
	COMMAND("remove <station>", "Remove a station from the list");
	COMMAND("rename <station> <name>", "Rename a station");
	COMMAND("move   <station> [[first/last] [before/after <station>]]", "");
	DESC   ("Move a station in the list");
	NL();

	TITLE  ("Configuration");
	print  (". sections: core, ui, feat.<feature-name>");
	COMMAND("conf get <section> <key>",         "Get a config value");
	COMMAND("conf set <section> <key> <value>", "Set a config value");
	COMMAND("conf list-keys <section>",         "List config keys");
	COMMAND("conf describe <section> <key>",    "Describe a config key");

	exit(exit_code);
}



/*
 * DBus
 */

#define DBUS_NAME           PACKAGE_APPLICATION_ID
#define DBUS_PATH           PACKAGE_APPLICATION_PATH
#define DBUS_ROOT_IFACE     PACKAGE_APPLICATION_ID
#define DBUS_PLAYER_IFACE   DBUS_ROOT_IFACE ".Player"
#define DBUS_STATIONS_IFACE DBUS_ROOT_IFACE ".Stations"

int
dbus_call(const char *bus_name,
          const char *object_path,
          const char *iface_name,
          const char *method_name,
          GVariant *args,
          GVariant **output)
{
	GDBusConnection *c;
	GVariant *result;
	GError *error = NULL;

	if (output)
		*output = NULL;

	c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
	if (c == NULL) {
		print_err("DBus connection error: %s", error->message);
		g_error_free(error);
		return -1;
	}

	result = g_dbus_connection_call_sync(c,
	                                     bus_name, object_path, iface_name, method_name,
	                                     args, NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL,
	                                     &error);

	if (error) {
		print_err("DBus call error: %s", error->message);
		g_error_free(error);
		return -1;
	}

	g_dbus_connection_close(c, NULL, NULL, NULL);

	if (output)
		*output = result;
	else if (result)
		g_variant_unref(result);

	return 0;
}



/*
 * Supported commands
 */

enum cmd_type {
	METHOD,
	PROPERTY
};

struct cmd {
	enum cmd_type type;
	const char *cmdline_name;
	const char *dbus_name;
	int  (*parse_args)   (int, char *[], GVariantBuilder *);
	void (*print_result) (GVariant *);
};

struct interface {
	const char *name;
	const struct cmd *cmds;
};

int
parse_play_args(int argc, char *argv[], GVariantBuilder *b)
{
	const char *station;

	if (argc == 0)
		station = "";
	else if (argc == 1)
		station = argv[0];
	else
		return -1;

	g_variant_builder_add(b, "s", station);

	return 0;
}

int
parse_add_args(int argc, char *argv[], GVariantBuilder *b)
{
	const char *station_uri;
	const char *station_name;
	const char *around_keyword;
	const char *around_station;

	if (argc == 0)
		return -1;

	station_uri = argv[0];
	argc--;
	argv++;

	station_name = "";
	if (argc > 0) {
		if (strcmp(argv[0], "first") && strcmp(argv[0], "last") &&
		    strcmp(argv[0], "before") && strcmp(argv[0], "after")) {
			station_name = argv[0];
			argc--;
			argv++;
		}
	}

	around_keyword = "";
	around_station = "";
	if (argc == 0) {
		/* Nothing to do */
	} else if (argc == 1) {
		if (!strcmp(argv[0], "first") || !strcmp(argv[0], "last"))
			around_keyword = argv[0];
		else
			return -1;
	} else if (argc == 2) {
		if (!strcmp(argv[0], "before") || !strcmp(argv[0], "after")) {
			around_keyword = argv[0];
			around_station = argv[1];
		} else {
			return -1;
		}
	} else {
		return -1;
	}

	g_variant_builder_add(b, "s", station_uri);
	g_variant_builder_add(b, "s", station_name);
	g_variant_builder_add(b, "s", around_keyword);
	g_variant_builder_add(b, "s", around_station);

	return 0;
}

int
parse_remove_args(int argc, char *argv[], GVariantBuilder *b)
{
	const char *station;

	if (argc != 1)
		return -1;

	station = argv[0];
	g_variant_builder_add(b, "s", station);

	return 0;
}

int
parse_rename_args(int argc, char *argv[], GVariantBuilder *b)
{
	const char *station;
	const char *new_name;

	if (argc != 2)
		return -1;

	station = argv[0];
	new_name = argv[1];

	g_variant_builder_add(b, "s", station);
	g_variant_builder_add(b, "s", new_name);

	return 0;
}

int
parse_move_args(int argc, char *argv[], GVariantBuilder *b)
{
	const char *station;
	const char *where;
	const char *around_station;

	if (argc < 2)
		return -1;

	station = argv[0];
	where = argv[1];
	argc -= 2;
	argv += 2;

	around_station = "";
	if (argc == 0) {
		if (strcmp(where, "first") && strcmp(where, "last"))
			return -1;
	} else if (argc == 1) {
		if (!strcmp(where, "before") || !strcmp(where, "after"))
			around_station = argv[0];
		else
			return -1;
	} else {
		return -1;
	}

	g_variant_builder_add(b, "s", station);
	g_variant_builder_add(b, "s", where);
	g_variant_builder_add(b, "s", around_station);

	return 0;
}

int
parse_boolean(int argc, char *argv[], GVariantBuilder *b)
{
	gboolean value;

	if (argc != 1)
		return -1;

	if (!g_strcmp0(argv[0], "true"))
		value = TRUE;
	else if (!g_strcmp0(argv[0], "false"))
		value = FALSE;
	else
		return -1;

	g_variant_builder_add(b, "v", g_variant_new_boolean(value));

	return 0;
}

int
parse_volume(int argc, char *argv[], GVariantBuilder *b)
{
	long int value;
	char *endptr;

	if (argc != 1)
		return -1;

	value = strtol(argv[0], &endptr, 10);
	if (*endptr != '\0')
		return -1;

	g_variant_builder_add(b, "v", g_variant_new_uint32(value));

	return 0;
}

void
print_boolean(GVariant *result)
{
	gboolean bool;

	bool = g_variant_get_boolean(result);

	if (bool)
		print("true");
	else
		print("false");
}

void
print_volume(GVariant *result)
{
	guint volume;

	volume = g_variant_get_uint32(result);
	print("%u%%", volume);
}

void
print_current(GVariant *result)
{
	GVariantIter *iter;
	GVariant *value;
	gchar *key;
	gchar *uri = NULL;
	gchar *name = NULL;
	gchar *artist = NULL;
	gchar *title = NULL;
	gchar *album = NULL;
	gchar *genre = NULL;
	gchar *year = NULL;
	gchar *comment = NULL;

	g_variant_get(result, "a{sv}", &iter);

	while (g_variant_iter_loop(iter, "{sv}", &key, &value)) {
		if (!g_strcmp0(key, "uri"))
			g_variant_get(value, "s", &uri);
		else if (!g_strcmp0(key, "name"))
			g_variant_get(value, "s", &name);
		else if (!g_strcmp0(key, "artist"))
			g_variant_get(value, "s", &artist);
		else if (!g_strcmp0(key, "title"))
			g_variant_get(value, "s", &title);
		else if (!g_strcmp0(key, "album"))
			g_variant_get(value, "s", &album);
		else if (!g_strcmp0(key, "genre"))
			g_variant_get(value, "s", &genre);
		else if (!g_strcmp0(key, "year"))
			g_variant_get(value, "s", &year);
		else if (!g_strcmp0(key, "comment"))
			g_variant_get(value, "s", &comment);
	}

	g_variant_iter_free(iter);

	/* Radio name and URI */
	print(BOLD("%s")"%s(%s)",
	      name,
	      name ? "\t" : "",
	      uri);

	/* Artist and title */
	if (artist || title)
		print(BOLD("> %s%s%s"),
		      title ? title : "",
		      title && artist ? " - " : "",
		      artist ? artist : "");

	/* Album, year and genre */
	if (genre) {
		gchar *tmp = genre;
		genre = g_strdup_printf("(%s)", tmp);
		g_free(tmp);
	}

	if (album || year || genre)
		print("%s%s%s%s%s",
		      album ? album : "",
		      album && year ? " " : "",
		      year ? year : "",
		      (album && genre) || (year && genre) ? " - " : "",
		      genre ? genre : "");

	/* Freedom for the braves */
	g_free(uri);
	g_free(name);
	g_free(artist);
	g_free(title);
	g_free(album);
	g_free(genre);
	g_free(year);
	g_free(comment);
}

void
print_list_result(GVariant *result)
{
	GVariantIter *iter1;
	GVariantIter *iter2;
	GVariant *value;
	gchar *key;

	g_variant_get(result, "(aa{sv})", &iter1);

	while (g_variant_iter_loop(iter1, "a{sv}", &iter2)) {
		gchar *uri = NULL;
		gchar *name = NULL;

		while (g_variant_iter_loop(iter2, "{sv}", &key, &value)) {
			if (!g_strcmp0(key, "uri"))
				g_variant_get(value, "s", &uri);
			else if (!g_strcmp0(key, "name"))
				g_variant_get(value, "s", &name);
		}

		print(BOLD("%-20s") "%s", name ? name : "", uri);

		g_free(uri);
		g_free(name);
	}

	g_variant_iter_free(iter1);
}

struct cmd root_cmds[] = {
	{ METHOD, "quit", "Quit", NULL, NULL },
	{ METHOD, NULL,   NULL,   NULL, NULL }
};

struct cmd player_cmds[] = {
	{ METHOD,   "play",     "Play",     parse_play_args, NULL          },
	{ METHOD,   "stop",     "Stop",     NULL,            NULL          },
	{ METHOD,   "next",     "Next",     NULL,            NULL          },
	{ METHOD,   "prev",     "Previous", NULL,            NULL          },
	{ METHOD,   "previous", "Previous", NULL,            NULL          },
	{ PROPERTY, "current",  "Current",  NULL,            print_current },
	{ PROPERTY, "playing",  "Playing",  NULL,            print_boolean },
	{ PROPERTY, "repeat",   "Repeat",   parse_boolean,   print_boolean },
	{ PROPERTY, "shuffle",  "Shuffle",  parse_boolean,   print_boolean },
	{ PROPERTY, "volume",   "Volume",   parse_volume,    print_volume  },
	{ PROPERTY, "mute",     "Mute",     parse_boolean,   print_boolean },
	{ PROPERTY, NULL,       NULL,       NULL,            NULL          }
};

struct cmd stations_cmds[] = {
	{ METHOD,   "list",    "List",   NULL,              print_list_result },
	{ METHOD,   "add",     "Add",    parse_add_args,    NULL              },
	{ METHOD,   "remove",  "Remove", parse_remove_args, NULL              },
	{ METHOD,   "rename",  "Rename", parse_rename_args, NULL              },
	{ METHOD,   "move",    "Move",   parse_move_args,   NULL              },
	{ METHOD,   NULL,      NULL,     NULL,              NULL              }
};

struct interface interfaces[] = {
	{ DBUS_ROOT_IFACE,     root_cmds     },
	{ DBUS_PLAYER_IFACE,   player_cmds   },
	{ DBUS_STATIONS_IFACE, stations_cmds },
	{ NULL,                NULL          }
};

static void
capitalize_first_letters(gchar *str)
{
	char *ptr;

	if (!str)
		return;

	*str = toupper(*str);
	str++;

	for (ptr = str; *ptr != '\0'; ptr++) {
		if (*ptr == '.') {
			ptr++;
			if (*ptr != '\0')
				*ptr = toupper(*ptr);
		}
	}
}

static int
handle_conf_command(int argc, char *argv[])
{
	const gchar *cmd;
	const gchar *section;
	const gchar *key;
	const gchar *value_str;
	gchar *schema_id;
	gchar *gsettings_cmd;
	int success;

	if (argc < 1)
		help_and_exit(EXIT_FAILURE);

	/* Command */
	cmd = argv[0];

	/* Section */
	capitalize_first_letters(argv[1]);
	section = argv[1];
	schema_id = g_strjoin(".", PACKAGE_APPLICATION_ID, section, NULL);

	argc -= 2;
	argv += 2;

	/* Process the other arguments */
	gsettings_cmd = NULL;
	if (!strcmp(cmd, "get")) {
		if (argc != 1)
			help_and_exit(EXIT_FAILURE);

		key = argv[0];

		gsettings_cmd = g_strdup_printf("gsettings get %s %s",
		                                schema_id, key);

	} else if (!strcmp(cmd, "set")) {
		if (argc != 2)
			help_and_exit(EXIT_FAILURE);

		key = argv[0];
		value_str = argv[1];

		gsettings_cmd = g_strdup_printf("gsettings set %s %s %s",
		                                schema_id, key, value_str);

	} else if (!strcmp(cmd, "list-keys")) {
		if (argc != 0)
			help_and_exit(EXIT_FAILURE);

		gsettings_cmd = g_strdup_printf("gsettings list-keys %s",
		                                schema_id);

	} else if (!strcmp(cmd, "describe")) {
		if (argc != 1)
			help_and_exit(EXIT_FAILURE);

		key = argv[0];

		gsettings_cmd = g_strdup_printf("gsettings describe %s %s",
		                                schema_id, key);

	} else {
		help_and_exit(EXIT_FAILURE);
	}

	success = system(gsettings_cmd);

	g_free(gsettings_cmd);
	g_free(schema_id);

	return success;
}

int
main(int argc, char *argv[])
{
	struct interface *iface;
	const struct cmd *cmd;
	GVariant *args, *result;
	int err;

	err = 0;

	help_init(argv[0]);

	if (argc < 2)
		help_and_exit(EXIT_FAILURE);

	/* Handle special commands */
	if (!strcmp(argv[1], "launch")) {
		/* Launch the application through DBus */
		GVariantBuilder b;
		g_variant_builder_init(&b, G_VARIANT_TYPE_TUPLE);
		g_variant_builder_add(&b, "s", DBUS_NAME);
		g_variant_builder_add(&b, "u", 0);
		args = g_variant_builder_end(&b);

		err = dbus_call("org.freedesktop.DBus", "/org/freedesktop/DBus",
		                "org.freedesktop.DBus", "StartServiceByName",
		                args, NULL);

		exit(err ? EXIT_FAILURE : EXIT_SUCCESS);

	} else if (!strcmp(argv[1], "conf")) {
		argc -= 2;
		argv += 2;

		err = handle_conf_command(argc, argv);
		exit(err ? EXIT_FAILURE : EXIT_SUCCESS);

	} else if (!strcmp(argv[1], "help")) {
		help_and_exit(EXIT_SUCCESS);
	}

	/* Find command in lists */
	for (iface = interfaces; iface->name; iface++) {
		for (cmd = iface->cmds; cmd->cmdline_name; cmd++) {
			if (!strcmp(argv[1], cmd->cmdline_name))
				break;
		}
		if (cmd->cmdline_name)
			break;
	}

	if (iface->name == NULL)
		help_and_exit(EXIT_FAILURE);

	/* Discard arguments that has been processed */
	argc -= 2;
	argv += 2;

	/* Process arguments left */
	args = NULL;
	switch (cmd->type) {
	case METHOD:
		/* For methods, if there's a parse function provided, we run it,
		 * no matter the number of arguments left.
		 */
		if (cmd->parse_args) {
			GVariantBuilder b;
			g_variant_builder_init(&b, G_VARIANT_TYPE_TUPLE);
			err = cmd->parse_args(argc, argv, &b);
			args = g_variant_builder_end(&b);
		} else if (argc > 0) {
			help_and_exit(EXIT_FAILURE);
		}
		break;

	case PROPERTY: {
		/* For properties, it's the number of remaining argument which
		 * determines if it's a get or a set. Zero argument means get.
		 */
		GVariantBuilder b;
		g_variant_builder_init(&b, G_VARIANT_TYPE_TUPLE);
		g_variant_builder_add(&b, "s", iface->name);
		g_variant_builder_add(&b, "s", cmd->dbus_name);

		if (argc > 0) {
			if (cmd->parse_args)
				err = cmd->parse_args(argc, argv, &b);
			else
				help_and_exit(EXIT_FAILURE);
		}

		args = g_variant_builder_end(&b);

		break;
	}
	}

	if (err)
		help_and_exit(EXIT_FAILURE);

	/* DBus action (method call, property get/set) */
	result = NULL;
	switch (cmd->type) {
	case METHOD:
		err = dbus_call(DBUS_NAME, DBUS_PATH, iface->name,
		                cmd->dbus_name, args, &result);
		break;
	case PROPERTY:
		if (argc == 0)
			/* Get command */
			err = dbus_call(DBUS_NAME, DBUS_PATH,
			                "org.freedesktop.DBus.Properties",
			                "Get", args, &result);
		else
			/* Set command */
			err = dbus_call(DBUS_NAME, DBUS_PATH,
			                "org.freedesktop.DBus.Properties",
			                "Set", args, NULL);
		break;
	}

	if (err)
		exit(EXIT_FAILURE);

	/* Print result */
	if (result && cmd->print_result) {
		// print("%s", g_variant_print(result, FALSE));

		if (cmd->type == METHOD) {
			cmd->print_result(result);
		} else {
			/* cmd->type == PROPERTY */
			/* Result is always a GVariant, encapsulated in a tuple */
			GVariant *tmp;
			g_variant_get(result, "(v)", &tmp);
			cmd->print_result(tmp);
			g_variant_unref(tmp);
		}
	}

	if (result)
		g_variant_unref(result);

	return EXIT_SUCCESS;
}
