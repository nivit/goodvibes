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

Please note that my first goal for Overcooked is to ensure a long-term
maintenance. There are more than enough software with a short lifetime around,
and I don't want to add one more to the list. I don't want to let my users down
in a year or so. I sincerely wish that Overcooked make it through the years,
and users can rely on it to listen internet radios.

Therefore, clean design and zero hacks is my priority. Dirty hacks are OK to
solve a problem quickly, or to demonstrate a possible solution. But they are
not OK for long-term maintenance, and therefore won't make it upstream.



Design Overview
---------------

Overcooked is written in C/GObject. The code is heavily object-oriented, and if
you're not familiar with GObject, you will have to accept that some magic is
going on, here and there. Good luck with that ;)

The code is neatly split into different parts:

- `additions`: this is where I extend some of the libraries I use, where I add
  some functions that I wish would exist already.
- `core`: the core of Overcooked, basically enough to have the software up and
  running, without the ui.
- `framework`: the name says it all.
- `libgszn`: a standalone library for GObject serialization. It is used to read
  and write the configuration file, and the stations list.
- `ui`: the GTK+ user interface.

Moreover, `core` and `ui` contain a `feat` subdirectory: here, you will find
some features that can be enabled/disabled at compile-time. I guess it's quite
similar to the plugins you often find in music players on GNU/Linux. Except
that I didn't dare to call it plugin, for plugins are usually something
discovered and loaded at run-time, not compile-time.

I suggest to have a look at `configure.ac` and `src/Makefile.am` for more
details about how things are sorted and separated.



Program invocation
------------------

To get a brief overview of the command-line options available, invoke with `-h`.

The option you will use the most is `-l` to change the log level. Colors are
enabled by default, but you can disable it with `-c` if it hurts your eyes.
Colors are automatically disabled when logs are redirected to a file.

Logs are all sent to `stderr`, whatever the log level.

Internally, we use the GLib to ouput log messages. For more details, refer to:

- [GLib Message Output and Debugging Functions](https://developer.gnome.org/glib/stable/glib-Message-Logging.html)

Some of the libraries we use provide additional command-line options. To see
them all, invoke with `--help-all`. For more details, refer to:

- [Running GLib Applications](https://developer.gnome.org/glib/stable/glib-running.html)
- [Running GStreamer Applications](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gst-running.html)
- [Running GTK+ Applications](https://developer.gnome.org/gtk3/stable/gtk-running.html)



Coding style
------------

Coding style matters to me. I keep the codebase as clean and neat as possible.
I'm a bit of a maniac about that, I tell you.

### Indentation

The code is currently indented using [Artistic Style](http://astyle.sourceforge.net/).
There's a script to automatically indent the whole thing:

	./scripts/code/indent.sh all

You can also easily indent your staged changes only before commiting:

	./scripts/code/indent.sh staged

### Comments & codetags

For comments, always use C style (aka `/* ... */`).

For codetags, always use C++ style (aka `//`).

Here are the codetags in use here:

- WISHED Things I wish, but might never be done.
- TODO   Things that should be done pretty soon.
- FIXME  For things obviously broken.

Always try to make it a one-liner if possible.

Stick to these conventions, and then getting a TODO list becomes very quick:

	ack 'TODO'

Alternatively, getting the list of things to be done:

	ack '// '



GObject C file layout
---------------------

If you find yourself writing a new file, therefore creating a new object, you
might want to use a script that generates all the boilerplate.

	./scripts/code/ock-object-make.sh

Have a look at the structure of the C file created, and please notice a few
things.

Functions are ordered from low-level to high-level, to avoid the need to
declare functions before defining it. So basically, you read the file from
bottom to top.

Functions are grouped in sections, titled by a comment such as:

	/*
	 * Section name
	 */

Most of the time, these sections are always the same, because implementing a
GObject always boils down to the same thing, more or less: GObject inherited
methods, property accessors, signal handlers, public methods, private methods,
helpers. Sometimes we implement an interface, and that's all there is to it.
So I try to show this consistent layout and this simplicity in the C file, and
to have it always the same way, instead of throwing code in a complete mess.

If you stick to these conventions, then it's easy to find your way in the code.
Whatever file you're looking at, doesn't matter, they all look the same.



GObject implementation conventions
----------------------------------

When implementing a GObject, there are some good practices to follow.
First because we want to listen to the recommendations from the wise guys,
and write some good code. Second, because we want the whole code to be
consistent, and look the same everywhere.

For an introduction, please refer to:

- [GObject Conventions](https://developer.gnome.org/gobject/stable/gtype-conventions.html)

### Use `G_DECLARE_*` and `G_DEFINE_*`

Do not clutter the header file with too much boilerplate, as it's often seen in
some old GObject-based code. Reference:

- [G_DECLARE_FINAL_DERIVABLE_TYPE](https://blogs.gnome.org/desrt/2015/01/27/g_declare_finalderivable_type/)

### Do the minimum in `_init()`

There should be almost nothing in the `init` function. Most of the time, it just
boils down to setting up the private pointer, if any.

For all the rest, it's better to implement the `constructed` method. Reference:

- [a gentle introduction to gobject construction](https://blogs.gnome.org/desrt/2012/02/26/a-gentle-introduction-to-gobject-construction/)

### Always use a private pointer

You will notice that in here, we never put anything in the object structure.
We ALWAYS use a private structure. And we always have a pointer named `priv`
to access it.

It's a very common convention, and once again, doing it everywhere, instead of
here and there, makes the code more consistent. Reference:

- [Changing quickly between a final and derivable GObject class](https://blogs.gnome.org/swilmet/2015/10/10/changing-quickly-between-a-final-and-derivable-gobject-class/)

