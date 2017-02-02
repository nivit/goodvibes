EXTRA_DIST     =
DISTCLEANFILES =
CLEANFILES     =

# ----------------------------------------------------- #
#         D-Bus Service File                            #
# ----------------------------------------------------- #

servicedir       = $(datadir)/dbus-1/services
service_in_files = @PACKAGE_APPLICATION_ID@.service.in
service_DATA     = $(service_in_files:.in=)

EXTRA_DIST     += $(service_in_files)
DISTCLEANFILES += $(service_DATA)

# ----------------------------------------------------- #
#         Desktop File                                  #
# ----------------------------------------------------- #

desktopdir          = $(datadir)/applications
desktop_in_in_files = @PACKAGE_APPLICATION_ID@.desktop.in.in
desktop_in_files    = $(desktop_in_in_files:.in=)
desktop_DATA        = $(desktop_in_files:.in=)

# Translate with gettext
%.desktop: %.desktop.in
	msgfmt --desktop -d $(top_srcdir)/po --template $< -o $@ 

EXTRA_DIST     += $(desktop_in_in_files)
DISTCLEANFILES += $(desktop_in_files)
CLEANFILES     += $(desktop_DATA)

# ----------------------------------------------------- #
#         GSettings Schemas                             #
# ----------------------------------------------------- #

gsettings_ENUM_NAMESPACE = @PACKAGE_APPLICATION_ID@
gsettings_ENUM_FILES     = $(top_srcdir)/src/*/*.h
gsettings_SCHEMAS        = @PACKAGE_APPLICATION_ID@.gschema.xml

@GSETTINGS_RULES@

# Compile schemas for running from the source tree
gschemas.compiled: $(gsettings_SCHEMAS:.xml=.valid)
	$(AM_V_GEN) $(GLIB_COMPILE_SCHEMAS) .

all-local: gschemas.compiled

EXTRA_DIST     += $(gsettings_SCHEMAS).in
DISTCLEANFILES += $(gsettings_SCHEMAS)
CLEANFILES     += gschemas.compiled

# ----------------------------------------------------- #
#         Icons                                         #
# ----------------------------------------------------- #

icon16dir    = $(datadir)/icons/hicolor/16x16/apps
icon16_DATA  = icons/hicolor/16x16/apps/@PACKAGE_NAME@.png
icon22dir    = $(datadir)/icons/hicolor/22x22/apps
icon22_DATA  = icons/hicolor/22x22/apps/@PACKAGE_NAME@.png
icon24dir    = $(datadir)/icons/hicolor/24x24/apps
icon24_DATA  = icons/hicolor/24x24/apps/@PACKAGE_NAME@.png
icon32dir    = $(datadir)/icons/hicolor/32x32/apps
icon32_DATA  = icons/hicolor/32x32/apps/@PACKAGE_NAME@.png
icon48dir    = $(datadir)/icons/hicolor/48x48/apps
icon48_DATA  = icons/hicolor/48x48/apps/@PACKAGE_NAME@.png
icon256dir   = $(datadir)/icons/hicolor/256x256/apps
icon256_DATA = icons/hicolor/256x256/apps/@PACKAGE_NAME@.png

EXTRA_DIST +=		\
	$(icon16_DATA)	\
	$(icon22_DATA)	\
	$(icon24_DATA)	\
	$(icon32_DATA)	\
	$(icon48_DATA)	\
	$(icon256_DATA)	\
	icons/src/@PACKAGE_NAME@.svg

# ----------------------------------------------------- #
#         Ui Files                                      #
# ----------------------------------------------------- #

uidir   = $(pkgdatadir)/ui
ui_DATA = 			\
	ui/app-menu.glade	\
	ui/main-window.glade	\
	ui/menubar.glade	\
	ui/prefs-window.glade	\
	ui/station-dialog.glade

EXTRA_DIST += $(ui_DATA)
