Goodvibes Radio Player
======================

* [Installation](#installation)
* [License](#license)
* [Hacking](#hacking)



Goodvibes is a lightweight internet radio player for GNU/Linux. It offers a simple way to have your favorite radio stations at easy reach.

Goodvibes provides everything you can expect from a modern media player: multimedia keys binding, mpris2 support, notifications, and sleep inhibition. It can be launched with or without GUI, and comes with a command-line client.

Behind the hood, Goodvibes relies on proven and approved open-source libraries: [GLib][] at the core, [GStreamer][] to handle the audio, and [GTK+][] for the graphical user interface.

The project is hosted on Github at <https://github.com/elboulangero/goodvibes>.<br>
The documentation is available at <https://github.com/elboulangero/goodvibes/wiki>.

[glib]:      https://wiki.gnome.org/Projects/GLib
[gstreamer]: https://gstreamer.freedesktop.org/
[gtk+]:      https://www.gtk.org/



Installation
------------

The easiest way to install Goodvibes is to use an existing package for your distribution. You can check the [release][] pages to see what's available.

If there's no package for you, you will need to grab the source code at first.

	git clone https://github.com/elboulangero/goodvibes.git

Compilation is done with the [Autotools][] using the usual set of commands.

	./autogen.sh
	./configure
	make

Installation is a one-liner, and must be run as root.

	sudo make install

Of course, you need to install the required dependencies.

- For the core: `glib`, `libsoup`, `libxml2`, `gstreamer`
- For the UI: `gtk+`, `libkeybinder`, `libnotify`

To install all the dependencies under Debian, run:

	sudo apt install build-essential autoconf autopoint
	sudo apt install libglib2.0-dev libsoup2.4-dev libxml2-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
	sudo apt install libgtk-3-dev libkeybinder-3.0-dev libnotify-dev

For more details, please refer to the `configure.ac` file.

If you're a packager and want to help, please get in touch.

[release]:   https://github.com/elboulangero/goodvibes/releases
[autotools]: https://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html


License
-------

Copyright (C) 2015-2017 Arnaud Rebillout <elboulangero@gmail.com>

Goodvibes is released under the [General Public License (GPL) version 3](LICENSE).



Hacking
-------

Please refer to the [HACKING](HACKING.md) file to get started.
