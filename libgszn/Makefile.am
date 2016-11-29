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
	$(eval ns := __LIBGSZN_$(shell basename $(1) | tr a-z- A-Z_)_H__)
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
#                  Libgszn                              #
# ----------------------------------------------------- #

# Sources

libgszn_sources =					\
	gszn-backend.c		gszn-backend.h		\
	gszn-backend-xml.c	gszn-backend-xml.h	\
	gszn-backend-keyfile.c	gszn-backend-keyfile.h	\
	gszn.c			gszn.h			\
	gszn-deserializer.c	gszn-deserializer.h	\
	gszn-error.c		gszn-error.h		\
				gszn-param-specs.h	\
	gszn-serializer.c	gszn-serializer.h	\
	gszn-settings.c		gszn-settings.h		\
	gszn-shared.c		gszn-shared.h

# Generated types

libgszn_built_sources =					\
	gszn-enum-types.c	gszn-enum-types.h

libgszn_types_prereqs =					\
	gszn-serializer.h

$(eval $(call make_types,gszn-enum-types,$(libgszn_types_prereqs)))

# Compiler options

libgszn_cflags = 		\
	-DG_LOG_DOMAIN=\"Gszn\"	\
	$(GLIB_CFLAGS)		\
	$(XML_CFLAGS)



# ----------------------------------------------------- #
#                Static Library                         #
# ----------------------------------------------------- #

noinst_LIBRARIES = libgszn.a

libgszn_a_SOURCES = 		\
	$(libgszn_sources)	\
	$(libgszn_built_sources)

libgszn_a_CFLAGS = $(libgszn_cflags)

BUILT_SOURCES =	$(libgszn_built_sources)
CLEANFILES = $(BUILT_SOURCES)
