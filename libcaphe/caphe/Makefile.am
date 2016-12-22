# Process this file with automake to produce Makefile.in

# ----------------------------------------------------- #
#                  Libcaphe                             #
# ----------------------------------------------------- #

libcaphe_sources =							\
	caphe.c				caphe.h				\
	caphe-dbus-inhibitor.c		caphe-dbus-inhibitor.h		\
	caphe-dbus-invokator.c		caphe-dbus-invokator.h		\
	caphe-dbus-proxy.c		caphe-dbus-proxy.h		\
	caphe-inhibitor.c		caphe-inhibitor.h		\
	caphe-login-dbus-invokator.c	caphe-login-dbus-invokator.h	\
	caphe-main.c			caphe-main.h			\
	caphe-power-dbus-invokator.c	caphe-power-dbus-invokator.h	\
	caphe-session-dbus-invokator.c	caphe-session-dbus-invokator.h	\
	caphe-trace.h

libcaphe_cflags = 			\
	-DG_LOG_DOMAIN=\"Caphe\"	\
	$(GLIB_CFLAGS)

#libcaphe_cflags += -DENABLE_TRACE

# ----------------------------------------------------- #
#                Static Library                         #
# ----------------------------------------------------- #

noinst_LIBRARIES = libcaphe.a

libcaphe_a_SOURCES = $(libcaphe_sources)
libcaphe_a_CFLAGS  = $(libcaphe_cflags)
