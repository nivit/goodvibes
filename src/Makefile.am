# Process this file with automake to produce Makefile.in

# ----------------------------------------------------- #
#         Enum Types Generation Template                #
# ----------------------------------------------------- #

# There's a bit of Makefile Magic going on here.
# We define a template, that we invoke later on to generate rules
# dynamically. It saves quite a lot of annoying copy/pasting, right ?

define make_types
$(1).c: $(2)
	$$(AM_V_GEN) glib-mkenums 						\
	--fhead "#include \"$(1).h\"\n" 					\
	--fhead "\n" 								\
	--fprod "/* Enumerations from \"@filename@\" */\n"			\
	--fprod "\n"								\
	--fprod "#include \"@filename@\"\n"					\
	--fprod "\n"								\
	--vhead "GType\n"							\
	--vhead "@enum_name@_get_type(void)\n"					\
	--vhead "{\n"								\
	--vhead "	static GType etype = 0;\n"				\
	--vhead "\n"								\
	--vhead "	if (etype == 0) {\n"					\
	--vhead "		static const G@Type@Value values[] = {\n"	\
	--vprod "			{ @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" },\n"\
	--vtail "			{ 0, NULL, NULL }\n"			\
	--vtail "		};\n"						\
	--vtail "		etype = g_@type@_register_static(\"@EnumName@\", values);\n"\
	--vtail "	}\n"							\
	--vtail "\n"								\
	--vtail "	return  etype;\n"					\
	--vtail "}\n"								\
	$$^ > $$@

$(1).h: $(2)
	$(eval ns := __GOODVIBES_$(shell basename $(1) | tr a-z- A-Z_)_H__)
	$$(AM_V_GEN) glib-mkenums						\
	--fhead "#ifndef $(ns)\n"						\
	--fhead "#define $(ns)\n"						\
	--fhead "\n"								\
	--fhead "#include <glib-object.h>\n"					\
	--fhead "\n"								\
	--fprod "/* Enumerations from \"@filename@\" */\n"			\
	--fprod "\n"								\
	--vhead "GType @enum_name@_get_type(void) G_GNUC_CONST;\n"		\
	--vhead "#define @ENUMNAME@_ENUM_TYPE (@enum_name@_get_type())\n"	\
	--vhead "\n"								\
	--ftail "#endif /* $(ns) */\n"						\
	$$^ > $$@
endef



# ----------------------------------------------------- #
#                  Additions                            #
# ----------------------------------------------------- #

gv_additions_sources =					\
	additions/glib.c	additions/glib.h	\
	additions/glib-object.c	additions/glib-object.h	\
	additions/gst.c		additions/gst.h

if UI_ENABLED
gv_additions_sources +=				\
	additions/gtk.c		additions/gtk.h
endif



# ----------------------------------------------------- #
#                  Framework                            #
# ----------------------------------------------------- #

# Sources

gv_framework_sources =							\
	framework/log.c			framework/log.h			\
	framework/gv-errorable.c	framework/gv-errorable.h	\
	framework/gv-feature.c		framework/gv-feature.h		\
	framework/gv-file-helpers.c	framework/gv-file-helpers.h	\
	framework/gv-framework.c	framework/gv-framework.h	\
	framework/gv-list.c		framework/gv-list.h		\
					framework/gv-param-specs.h	\
	framework/uri-schemes.c		framework/uri-schemes.h		\
					framework/vt-codes.h


# Generated types

gv_framework_built_sources =			\
	framework/gv-framework-enum-types.c	\
	framework/gv-framework-enum-types.h

gv_framework_types_prereqs =			\
	framework/gv-feature.h

$(eval $(call make_types,framework/gv-framework-enum-types,$(gv_framework_types_prereqs)))

# Compiler options - handled in core compiler options



# ----------------------------------------------------- #
#                  Core                                 #
# ----------------------------------------------------- #

# Sources

gv_core_sources =					\
	core/gv-conf.c		core/gv-conf.h		\
	core/gv-core.c		core/gv-core.h		\
	core/gv-engine.c	core/gv-engine.h	\
	core/gv-metadata.c	core/gv-metadata.h	\
	core/gv-player.c	core/gv-player.h	\
	core/gv-playlist.c	core/gv-playlist.h	\
	core/gv-station.c	core/gv-station.h	\
	core/gv-station-list.c	core/gv-station-list.h

# Enum Types

gv_core_built_sources =			\
	core/gv-core-enum-types.c	\
	core/gv-core-enum-types.h

gv_core_types_prereqs =			\
	core/gv-engine.h		\
	core/gv-player.h

$(eval $(call make_types,core/gv-core-enum-types,$(gv_core_types_prereqs)))

# Compiler options

gv_core_cflags = 		\
	$(CAPHE_CFLAGS)		\
	$(GSZN_CFLAGS)		\
	$(GLIB_CFLAGS)		\
	$(XML_CFLAGS)		\
	$(LIBSOUP_CFLAGS)	\
	$(GST_CFLAGS)

gv_core_static_ldadd =		\
	$(CAPHE_LIBS)		\
	$(GSZN_LIBS)

gv_core_shared_ldadd =		\
	-lm			\
	$(GLIB_LIBS)		\
	$(XML_LIBS)		\
	$(LIBSOUP_LIBS)		\
	$(GST_LIBS)

# Features

if CONSOLE_OUTPUT_ENABLED
gv_core_sources += core/feat/gv-console-output.c core/feat/gv-console-output.h
gv_core_cflags  += -DCONSOLE_OUTPUT_ENABLED
endif

if DBUS_NATIVE_ENABLED
gv_core_sources += core/feat/gv-dbus-server.c core/feat/gv-dbus-server.h \
		    core/feat/gv-dbus-server-native.c core/feat/gv-dbus-server-native.h
gv_core_cflags  += -DDBUS_SERVER_NATIVE_ENABLED
endif

if DBUS_MPRIS2_ENABLED
if ! DBUS_NATIVE_ENABLED
gv_core_sources += core/feat/gv-dbus-server.c core/feat/gv-dbus-server.h
endif
gv_core_sources += core/feat/gv-dbus-server-mpris2.c core/feat/gv-dbus-server-mpris2.h
gv_core_cflags  += -DDBUS_SERVER_MPRIS2_ENABLED
endif

if INHIBITOR_ENABLED
gv_core_sources += core/feat/gv-inhibitor.c core/feat/gv-inhibitor.h
gv_core_cflags  += -DINHIBITOR_ENABLED
endif



# ----------------------------------------------------- #
#                  User Interface                       #
# ----------------------------------------------------- #

# Sources

if UI_ENABLED
gv_ui_sources =								\
					ui/global.h			\
	ui/gv-about-dialog.c		ui/gv-about-dialog.h		\
	ui/gv-builder-helpers.c		ui/gv-builder-helpers.h		\
	ui/gv-main-menu.c		ui/gv-main-menu.h		\
	ui/gv-main-window.c		ui/gv-main-window.h		\
	ui/gv-prefs-window.c		ui/gv-prefs-window.h		\
	ui/gv-station-context-menu.c	ui/gv-station-context-menu.h	\
	ui/gv-station-dialog.c		ui/gv-station-dialog.h		\
	ui/gv-stations-tree-view.c	ui/gv-stations-tree-view.h	\
	ui/gv-stock-icons.c		ui/gv-stock-icons.h		\
	ui/gv-tray.c			ui/gv-tray.h			\
	ui/gv-ui.c			ui/gv-ui.h

# Enum Types

gv_ui_built_sources =		\
	ui/gv-ui-enum-types.c	\
	ui/gv-ui-enum-types.h

gv_ui_types_prereqs =		\
	ui/gv-tray.h

$(eval $(call make_types,ui/gv-ui-enum-types,$(gv_ui_types_prereqs)))

# Compiler options

gv_ui_cflags       = $(UI_CFLAGS) -DUI_ENABLED
gv_ui_shared_ldadd = $(UI_LIBS)

# Features

if HOTKEYS_ENABLED
gv_ui_sources      += ui/feat/gv-hotkeys.c ui/feat/gv-hotkeys.h
gv_ui_cflags       += $(HOTKEYS_CFLAGS) -DHOTKEYS_ENABLED
gv_ui_shared_ldadd += $(HOTKEYS_LIBS)
endif

if NOTIFICATIONS_ENABLED
gv_ui_sources      += ui/feat/gv-notifications.c ui/feat/gv-notifications.h
gv_ui_cflags       += $(NOTIFICATIONS_CFLAGS) -DNOTIFICATIONS_ENABLED
gv_ui_shared_ldadd += $(NOTIFICATIONS_LIBS)
endif

endif # UI_ENABLED



# ----------------------------------------------------- #
#               Goodvibes Radio Player                  #
# ----------------------------------------------------- #

# We use sort() to remove duplicates in CFLAGS and LDADD.
# I know, it's not recommended to use sort() in Automake,
# but it works and does a wonderful job, so let it be.

# Also, notice in LDADD that the static libraries MUST come first,
# otherwise the link might fail. If you're interested in the matter,
# here's a bit of reading I recommend:
# http://eli.thegreenplace.net/2013/07/09/library-order-in-static-linking

bin_PROGRAMS = goodvibes

goodvibes_SOURCES =			\
	$(gv_additions_sources)		\
	$(gv_framework_sources)		\
	$(gv_framework_built_sources)	\
	$(gv_core_sources)		\
	$(gv_core_built_sources)	\
	$(gv_ui_sources)		\
	$(gv_ui_built_sources)

if UI_ENABLED
goodvibes_SOURCES += gv-graphical-application.c gv-graphical-application.h
else
goodvibes_SOURCES += gv-console-application.c gv-console-application.h
endif

goodvibes_SOURCES += 			\
	main.c				\
	options.c	options.h

goodvibes_CFLAGS  =			\
	-DLOCALE_DIR=\"$(localedir)\"

goodvibes_CFLAGS += $(sort		\
	$(gv_framework_cflags)		\
	$(gv_core_cflags)		\
	$(gv_ui_cflags)			\
	)

goodvibes_LDADD  = 			\
	$(gv_core_static_ldadd)

goodvibes_LDADD += $(sort		\
	$(gv_framework_shared_ldadd)	\
	$(gv_core_shared_ldadd)		\
	$(gv_ui_shared_ldadd)		\
	)

BUILT_SOURCES =				\
	$(gv_framework_built_sources)	\
	$(gv_core_built_sources)	\
	$(gv_ui_built_sources)

CLEANFILES = $(BUILT_SOURCES)

print-flags:
	@echo -- CPPFLAGS --
	@echo $(CPPFLAGS) $(AM_CPPFLAGS) $(goodvibes_CPPFLAGS)
	@echo -- CFLAGS --
	@echo $(CFLAGS) $(AM_CFLAGS) $(goodvibes_CFLAGS)
	@echo -- LDADD --
	@echo $(LDADD) $(AM_LDADD) $(goodvibes_LDADD)
	@echo -- done --
