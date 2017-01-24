# Process this file with autoconf

# Init autoconf
# 2.60: obsoleting plenty of stuff
AC_PREREQ([2.60])
AC_INIT([goodvibes], [0.1])
: ${CPPFLAGS="-std=gnu99 -Wall -Wextra -Wshadow"}

# Init automake
# 1.11: silent-rules
# 1.14: obsoleting AM_PROG_CC_C_O
AM_INIT_AUTOMAKE([1.14 foreign subdir-objects])
AM_SILENT_RULES([yes])

# Load our own set of m4 macros
AC_CONFIG_MACRO_DIRS([m4])

# Additional package information
GV_ADD_GLOBAL_VAR([PACKAGE_CAMEL_NAME],       ["Goodvibes"])
GV_ADD_GLOBAL_VAR([PACKAGE_LONG_NAME],        ["Goodvibes Radio Player"])
GV_ADD_GLOBAL_VAR([PACKAGE_APPLICATION_ID],   ["com.elboulangero.Goodvibes"])
GV_ADD_GLOBAL_VAR([PACKAGE_APPLICATION_PATH], ["/com/elboulangero/Goodvibes"])
GV_ADD_GLOBAL_VAR([PACKAGE_AUTHOR_NAME],      ["Arnaud Rebillout"])
GV_ADD_GLOBAL_VAR([PACKAGE_AUTHOR_EMAIL],     ["<elboulangero@gmail.com>"])
GV_ADD_GLOBAL_VAR([PACKAGE_COPYRIGHT],        ["Copyright (C) 2015-2017"])
GV_ADD_GLOBAL_VAR([PACKAGE_WEBSITE],          ["https://github.com/elboulangero/goodvibes"])

# ------------------------------------- #
#              Toolchain                #
# ------------------------------------- #

# Check for toolchain and install components

AS_ECHO(["---- Toolchain ----"])

AC_PROG_CC()
AC_PROG_INSTALL()
PKG_PROG_PKG_CONFIG()

# To build static libraries (libgszn, libcaphe)
AC_PROG_RANLIB()
AM_PROG_AR()

# For glib settings
GLIB_GSETTINGS()

# ------------------------------------- #
#      Internationalization (i18n)      #
# ------------------------------------- #

AS_ECHO(["---- I18n ----"])

# Init gettext
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.7])

# ------------------------------------- #
#         Mandatory dependencies        #
# ------------------------------------- #

# Check for core dependencies
# - gio-unix-2.0 is needed for libcaphe. In a perfect world, libcaphe
# (and therefore gio-unix-2.0) should depend on the inhibitor feature.
# - libxml-2.0 is needed for libgszn (which is a mandatory dependency)

AS_ECHO(["---- Core ----"])

PKG_CHECK_MODULES([GLIB],    [glib-2.0, gobject-2.0, gio-2.0, gio-unix-2.0 >= 2.44])
PKG_CHECK_MODULES([LIBSOUP], [libsoup-2.4 >= 2.42])
PKG_CHECK_MODULES([GST],     [gstreamer-1.0, gstreamer-base-1.0, gstreamer-audio-1.0 >= 1.4.4])
PKG_CHECK_MODULES([XML],     [libxml-2.0 >= 2.9.3])

# Libgszn flags
GSZN_CFLAGS="-I.."
GSZN_LIBS="../libgszn/libgszn.a"
AC_SUBST(GSZN_CFLAGS)
AC_SUBST(GSZN_LIBS)

# Libcaphe flags
CAPHE_CFLAGS="-I../libcaphe/"
CAPHE_LIBS="../libcaphe/caphe/libcaphe.a"
AC_SUBST(CAPHE_CFLAGS)
AC_SUBST(CAPHE_LIBS)

# ------------------------------------- #
#         Optional dependencies         #
# ------------------------------------- #

# The default behavior is as following:
# - a feature that doesn't require an extra library is enabled.
# - a feature that requires an extra library is set to 'auto',
#   and will be enabled only if the required library is found.

feat_noextra="yes"
feat_extra="auto"

# The default behavior can be modified with the '--enable-all' switch.
# If you want to use it, expect the following behavior:
# - --enable-all   Build a full-featured version (maximum dependencies).
#                  This will force the compilation of every feature,
#                  and fail the build if a dependency is not met.
# - --disable-all  Build a lightweight version (minimum dependencies).
#                  This will exclude every feature from the build.

AC_ARG_ENABLE(all, AS_HELP_STRING([--enable-all], [Enable every features]))

if test -n "$enable_all"; then
  feat_noextra="$enable_all"
  feat_extra="$enable_all"
fi

# For a higher degree of granularity, every feature has its own switch.
# If you want to use it, expect the following behavior:
# - --enable-FEATURE   Force the compilation of a feature, and fail
#                      the build if dependencies are not met.
# - --disable-FEATURE  Exclude a feature from the build.

# Check for ui dependencies
# Notice that the UI is managed as a feature here.

AS_ECHO(["---- Ui ----"])

GV_FEATURE_ENABLE([ui], [Gtk+ graphical user interface], [$feat_extra])
GV_FEATURE_PKG_CHECK([ui], [UI], [gtk+-3.0 >= 3.12.0])
AM_CONDITIONAL([UI_ENABLED], [test "$enable_ui" = "yes"])

# Check for core features dependencies

AS_ECHO(["---- Core Features ----"])

GV_FEATURE_ENABLE([console_output], [console output support], [$feat_noextra])
GV_FEATURE_ENABLE([dbus_server],    [dbus server support],    [$feat_noextra])
GV_FEATURE_ENABLE([inhibitor],      [inhibitor support],      [$feat_noextra])

AM_CONDITIONAL([CONSOLE_OUTPUT_ENABLED], [test "$enable_console_output" = "yes"])
AM_CONDITIONAL([DBUS_SERVER_ENABLED],    [test "$enable_dbus_server" = "yes"])
AM_CONDITIONAL([INHIBITOR_ENABLED],      [test "$enable_inhibitor" = "yes"])

# Check for ui features dependencies
# If the ui is disabled, ensure features are disabled as well.

AS_ECHO(["---- Ui Features ----"])

if test "$enable_ui" = "no"; then
  feat_noextra="no"
  feat_extra="no"
fi

GV_FEATURE_ENABLE([hotkeys],       [hotkeys support],       [$feat_extra])
GV_FEATURE_ENABLE([notifications], [notifications support], [$feat_noextra])

GV_FEATURE_PKG_CHECK([hotkeys],       [HOTKEYS],       [keybinder-3.0 >= 0.3])

AM_CONDITIONAL([HOTKEYS_ENABLED],       [test "$enable_hotkeys" = "yes"])
AM_CONDITIONAL([NOTIFICATIONS_ENABLED], [test "$enable_notifications" = "yes"])

# ------------------------------------- #
#              Output files             #
# ------------------------------------- #

AS_ECHO(["---- Output Files ----"])

# Files to be created
AC_CONFIG_FILES([
AUTHORS
Makefile
data/Makefile
data/dbus-1/Makefile
data/dbus-1/services/Makefile
data/dbus-1/services/$PACKAGE_APPLICATION_ID.service
data/desktop/Makefile
data/desktop/$PACKAGE_APPLICATION_ID.desktop.in
data/glib-2.0/Makefile
data/glib-2.0/schemas/Makefile
data/glib-2.0/schemas/$PACKAGE_APPLICATION_ID.gschema.xml
data/icons/Makefile
data/icons/hicolor/Makefile
data/icons/hicolor/16x16/Makefile
data/icons/hicolor/16x16/apps/Makefile
data/icons/hicolor/22x22/Makefile
data/icons/hicolor/22x22/apps/Makefile
data/icons/hicolor/24x24/Makefile
data/icons/hicolor/24x24/apps/Makefile
data/icons/hicolor/32x32/Makefile
data/icons/hicolor/32x32/apps/Makefile
data/icons/hicolor/48x48/Makefile
data/icons/hicolor/48x48/apps/Makefile
data/icons/hicolor/256x256/Makefile
data/icons/hicolor/256x256/apps/Makefile
data/icons/src/Makefile
data/ui/Makefile
libcaphe/Makefile
libcaphe/caphe/Makefile
libcaphe/tests/Makefile
libgszn/Makefile
po/Makefile.in
src/Makefile
])

AC_OUTPUT()

# ------------------------------------- #
#                Summary                #
# ------------------------------------- #

# String for silent rules
is_silent_rules_enabled=no
if test "$AM_DEFAULT_VERBOSITY" = "0"; then
   is_silent_rules_enabled=yes
fi

AS_ECHO(["
-------------------------------------------------------------

	Goodvibes Radio Player
	======================

	Core
	----
	Console output    : $enable_console_output
	D-Bus server      : $enable_dbus_server
	Inhibitor         : $enable_inhibitor

	Ui                : $enable_ui
	--
	Hotkeys           : $enable_hotkeys
	Notifications     : $enable_notifications

	Build
	-----
	Silent rules      : $is_silent_rules_enabled
	CC                : $CC
	CFLAGS            : $CFLAGS
	CPPFLAGS          : $CPPFLAGS
	LDFLAGS           : $LDFLAGS
	LIBS              : $LIBS

-------------------------------------------------------------

Configuration finished, please type 'make' to compile."])

