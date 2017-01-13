%.desktop: %.desktop.in
	msgfmt --desktop -d $(top_srcdir)/po --template $< -o $@ 

desktopdir = $(datadir)/applications
desktop_in_in_files = @PACKAGE_APPLICATION_ID@.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.in=)
desktop_DATA = $(desktop_in_files:.in=)

EXTRA_DIST = $(desktop_in_in_files)
DISTCLEAN  = $(desktop_in_files)
CLEANFILES = $(desktop_DATA)
