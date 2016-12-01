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
	$(eval ns := __OVERCOOKED_$(shell basename $(1) | tr a-z- A-Z_)_H__)
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

ock_additions_sources =					\
	additions/glib.c	additions/glib.h	\
	additions/glib-object.c	additions/glib-object.h	\
	additions/gst.c		additions/gst.h

if UI_ENABLED
ock_additions_sources +=				\
	additions/gtk.c		additions/gtk.h
endif



# ----------------------------------------------------- #
#                  Framework                            #
# ----------------------------------------------------- #

# Sources

ock_framework_sources =							\
	framework/log.c			framework/log.h			\
	framework/ock-errorable.c	framework/ock-errorable.h	\
	framework/ock-feature.c		framework/ock-feature.h		\
	framework/ock-file-helpers.c	framework/ock-file-helpers.h	\
	framework/ock-framework.c	framework/ock-framework.h	\
	framework/ock-list.c		framework/ock-list.h		\
					framework/ock-param-specs.h	\
	framework/uri-schemes.c		framework/uri-schemes.h		\
					framework/vt-codes.h


# Generated types

ock_framework_built_sources =			\
	framework/ock-framework-enum-types.c	\
	framework/ock-framework-enum-types.h

ock_framework_types_prereqs =			\
	framework/ock-feature.h

$(eval $(call make_types,framework/ock-framework-enum-types,$(ock_framework_types_prereqs)))

# Compiler options - handled in core compiler options



# ----------------------------------------------------- #
#                  Core                                 #
# ----------------------------------------------------- #

# Sources

ock_core_sources =					\
	core/ock-conf.c		core/ock-conf.h		\
	core/ock-core.c		core/ock-core.h		\
	core/ock-engine.c	core/ock-engine.h	\
	core/ock-metadata.c	core/ock-metadata.h	\
	core/ock-player.c	core/ock-player.h	\
	core/ock-playlist.c	core/ock-playlist.h	\
	core/ock-station.c	core/ock-station.h	\
	core/ock-station-list.c	core/ock-station-list.h

# Enum Types

ock_core_built_sources =		\
	core/ock-core-enum-types.c	\
	core/ock-core-enum-types.h

ock_core_types_prereqs =		\
	core/ock-engine.h		\
	core/ock-player.h

$(eval $(call make_types,core/ock-core-enum-types,$(ock_core_types_prereqs)))

# Compiler options

ock_core_cflags = 		\
	$(GSZN_CFLAGS)		\
	$(GLIB_CFLAGS)		\
	$(XML_CFLAGS)		\
	$(LIBSOUP_CFLAGS)	\
	$(GST_CFLAGS)

ock_core_ldadd =		\
	-lm			\
	$(GSZN_LIBS)		\
	$(GLIB_LIBS)		\
	$(XML_LIBS)		\
	$(LIBSOUP_LIBS)		\
	$(GST_LIBS)

# Features

if CONSOLE_OUTPUT_ENABLED
ock_core_sources += core/feat/ock-console-output.c core/feat/ock-console-output.h
ock_core_cflags  += -DCONSOLE_OUTPUT_ENABLED
endif

if DBUS_NATIVE_ENABLED
ock_core_sources += core/feat/ock-dbus-server.c core/feat/ock-dbus-server.h \
		    core/feat/ock-dbus-server-native.c core/feat/ock-dbus-server-native.h
ock_core_cflags  += -DDBUS_SERVER_NATIVE_ENABLED
endif

if DBUS_MPRIS2_ENABLED
if ! DBUS_NATIVE_ENABLED
ock_core_sources += core/feat/ock-dbus-server.c core/feat/ock-dbus-server.h
endif
ock_core_sources += core/feat/ock-dbus-server-mpris2.c core/feat/ock-dbus-server-mpris2.h
ock_core_cflags  += -DDBUS_SERVER_MPRIS2_ENABLED
endif

if INHIBIT_ENABLED
ock_core_sources += core/feat/ock-inhibitor.c core/feat/ock-inhibitor.h
ock_core_cflags  += $(INHIBIT_CFLAGS) -DINHIBIT_ENABLED
ock_core_ldadd   += $(INHIBIT_LIBS)
endif



# ----------------------------------------------------- #
#                  User Interface                       #
# ----------------------------------------------------- #

# Sources

if UI_ENABLED
ock_ui_sources =							\
					ui/global.h			\
	ui/ock-about-dialog.c		ui/ock-about-dialog.h		\
	ui/ock-builder-helpers.c	ui/ock-builder-helpers.h	\
	ui/ock-main-menu.c		ui/ock-main-menu.h		\
	ui/ock-main-window.c		ui/ock-main-window.h		\
	ui/ock-prefs-window.c		ui/ock-prefs-window.h		\
	ui/ock-station-context-menu.c	ui/ock-station-context-menu.h	\
	ui/ock-station-dialog.c		ui/ock-station-dialog.h		\
	ui/ock-stations-tree-view.c	ui/ock-stations-tree-view.h	\
	ui/ock-stock-icons.c		ui/ock-stock-icons.h		\
	ui/ock-tray.c			ui/ock-tray.h			\
	ui/ock-ui.c			ui/ock-ui.h

# Enum Types

ock_ui_built_sources =		\
	ui/ock-ui-enum-types.c	\
	ui/ock-ui-enum-types.h

ock_ui_types_prereqs =		\
	ui/ock-tray.h

$(eval $(call make_types,ui/ock-ui-enum-types,$(ock_ui_types_prereqs)))

# Compiler options

ock_ui_cflags = $(UI_CFLAGS) -DUI_ENABLED
ock_ui_ldadd  = $(UI_LIBS)

# Features

if HOTKEYS_ENABLED
ock_ui_sources += ui/feat/ock-hotkeys.c	ui/feat/ock-hotkeys.h
ock_ui_cflags  += $(HOTKEYS_CFLAGS) -DHOTKEYS_ENABLED
ock_ui_ldadd   += $(HOTKEYS_LIBS)
endif

if NOTIFICATIONS_ENABLED
ock_ui_sources += ui/feat/ock-notifications.c ui/feat/ock-notifications.h
ock_ui_cflags  += $(NOTIFICATIONS_CFLAGS) -DNOTIFICATIONS_ENABLED
ock_ui_ldadd   += $(NOTIFICATIONS_LIBS)
endif

endif # UI_ENABLED



# ----------------------------------------------------- #
#                  Overcooked                           #
# ----------------------------------------------------- #

bin_PROGRAMS = overcooked

#
# Sources
# We use sort() to remove duplicates in CFLAGS and LDADD.
# I know, it's not recommended to use sort in Automake,
# but it works and does a wonderful job, so let it be.
#

overcooked_SOURCES =			\
	main.c				\
	options.c	options.h	\
	$(ock_additions_sources)	\
	$(ock_framework_sources)	\
	$(ock_framework_built_sources)	\
	$(ock_core_sources)		\
	$(ock_core_built_sources)	\
	$(ock_ui_sources)		\
	$(ock_ui_built_sources)

overcooked_CFLAGS = $(sort	\
	$(ock_framework_cflags)	\
	$(ock_core_cflags)	\
	$(ock_ui_cflags)	\
	)

overcooked_LDADD = $(sort	\
	$(ock_framework_ldadd)	\
	$(ock_core_ldadd)	\
	$(ock_ui_ldadd)		\
	)

BUILT_SOURCES =				\
	$(ock_framework_built_sources)	\
	$(ock_core_built_sources)	\
	$(ock_ui_built_sources)

CLEANFILES =			\
	$(BUILT_SOURCES)

print-flags:
	@echo -- CPPFLAGS --
	@echo $(CPPFLAGS) $(AM_CPPFLAGS) $(overcooked_CPPFLAGS)
	@echo -- CFLAGS --
	@echo $(CFLAGS) $(AM_CFLAGS) $(overcooked_CFLAGS)
	@echo -- LDADD --
	@echo $(LDADD) $(AM_LDADD) $(overcooked_LDADD)
	@echo -- done --
