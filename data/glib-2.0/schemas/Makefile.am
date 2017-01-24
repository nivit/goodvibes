gsettings_ENUM_NAMESPACE = @PACKAGE_APPLICATION_ID@
gsettings_ENUM_FILES     = $(top_srcdir)/src/*/*.h
gsettings_SCHEMAS        = @PACKAGE_APPLICATION_ID@.gschema.xml

EXTRA_DIST = $(gsettings_SCHEMAS).in
DISTCLEAN  = $(gsettings_SCHEMAS)

@GSETTINGS_RULES@

# Compile schemas for running from the source tree
gschemas.compiled: $(gsettings_SCHEMAS:.xml=.valid)
	$(AM_V_GEN) $(GLIB_COMPILE_SCHEMAS) .

all-local: gschemas.compiled

CLEANFILES = gschemas.compiled
