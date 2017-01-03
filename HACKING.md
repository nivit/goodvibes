HACKING
=======

Welcome hackers, here is the place to get started if you want to hack around !



Contributing
------------

Contributing is better done through Github, so please follow the usual Github
workflow:

- have your account setup
- fork the project
- work on your own version
- discuss the changes, and eventually submit a merge request

If you hack around for your own use-case, and think that other users could
benefit from your hacks, feel free to share it and discuss it on Github.

If we decide that your hacks should be integrated upstream, then you will have
to transform it in proper commits, make sure it fits into the current design,
that it respects the coding style and so on.

Please note that my first goal for Goodvibes is to ensure a long-term
maintenance. There are more than enough software with a short lifetime around,
and I don't want to add one more to the list. I don't want to let my users down
in a year or so. I sincerely wish that Goodvibes make it through the years,
and users can rely on it to listen internet radios.

Therefore, clean design and zero hacks is my priority. Dirty hacks are OK to
solve a problem quickly, or to demonstrate a possible solution. But they are
not OK for long-term maintenance, and therefore won't make it upstream.



Design Overview
---------------

Goodvibes is written in C/GObject. The code is heavily object-oriented, and if
you're not familiar with GObject, you will have to accept that some magic is
going on, here and there. Good luck with that ;)

The code is neatly split into different parts:

- `additions`: this is where I extend some of the libraries I use, where I add
  some functions that I wish would exist already.
- `core`: the core of Goodvibes, basically enough to have the software up and
  running, without the ui.
- `framework`: the name says it all.
- `libgszn`: a standalone library for GObject serialization. It is used to read
  and write the configuration file, and the station list file.
- `ui`: the GTK+ user interface.

Moreover, `core` and `ui` contain a `feat` subdirectory: here, you will find
some features that can be enabled/disabled at compile-time. I guess it's quite
similar to the plugins you often find in music players on GNU/Linux. Except
that I didn't dare to call it plugin, for plugins are usually something
discovered and loaded at run-time, not compile-time.

I suggest to have a look at `configure.ac` and `src/Makefile.am` for more
details about how things are sorted and separated.



Program Invocation
------------------

To get a brief overview of the command-line options available, invoke with `-h`.

The option you will use the most is `-l` to change the log level. Here is a typical line:

	./src/goodvibes -l trace

Colors are enabled by default, but you can disable it with `-c` if it hurts your eyes.
Colors are automatically disabled when logs are redirected to a file.

Logs are all sent to `stderr`, whatever the log level.

Internally, we use the GLib to ouput log messages. For more details, refer to:

- [GLib Message Output and Debugging Functions](https://developer.gnome.org/glib/stable/glib-Message-Logging.html)

Some of the libraries we use provide additional command-line options. To see
them all, invoke with `--help-all`. For more details, refer to:

- [Running GLib Applications](https://developer.gnome.org/glib/stable/glib-running.html)
- [Running GStreamer Applications](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gst-running.html)
- [Running GTK+ Applications](https://developer.gnome.org/gtk3/stable/gtk-running.html)

Hardcore GTK+ debugging can be done with [GtkInspector](https://wiki.gnome.org/Projects/GTK+/Inspector):

	./src/goodvibes --gtk-debug=interactive



Getting your hands dirty
------------------------

More doc is available in the [docs](docs) subdirectory. Please refer to:

- [coding-style.md](docs/coding-style.md) for everything related to coding style.
- [gobject.md](docs/gobject.md) for GObject conventions and good practices.
