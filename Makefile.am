SUBDIRS =	\
	cli	\
	data	\
	libgszn	\
	po	\
	src

EXTRA_DIST = 		\
	autogen.sh	\
	docs		\
	HACKING.md	\
	README.md	\
	THANKS.md

distclean-local:
	find . -name '*~' -print0 | \
		xargs --no-run-if-empty -0 rm -vf

indent:
	./scripts/code/indent.sh all

icons:
	inkscape -D -w16 -e data/icons/hicolor/16x16/apps/overcooked.png \
	  data/icons/src/overcooked.svg
	inkscape -D -w22 -e data/icons/hicolor/22x22/apps/overcooked.png \
	  data/icons/src/overcooked.svg
	inkscape -D -w24 -e data/icons/hicolor/24x24/apps/overcooked.png \
	  data/icons/src/overcooked.svg
	inkscape -D -w32 -e data/icons/hicolor/32x32/apps/overcooked.png \
	  data/icons/src/overcooked.svg
	inkscape -D -w48 -e data/icons/hicolor/48x48/apps/overcooked.png \
	  data/icons/src/overcooked.svg
	inkscape -D -w256 -e data/icons/hicolor/256x256/apps/overcooked.png \
	  data/icons/src/overcooked.svg
