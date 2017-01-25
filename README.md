Goodvibes Radio Player
======================

* [Installation](#installation)
* [Dependencies](#dependencies)
* [License](#license)
* [Translations](#translations)
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

[release]:   https://github.com/elboulangero/goodvibes/releases
[autotools]: https://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html



Dependencies
------------

Compile-time dependencies are:

- For the core: `glib`, `libsoup`, `gstreamer`
- For the user interface: `gtk+`, `libkeybinder`

Additional run-time dependencies are listed below.

For more gory details, please refer to the file [configure.ac](configure.ac).

#### Install dependencies on Debian

Compile-time dependencies:

	sudo apt install build-essential autoconf autopoint
	sudo apt install libglib2.0-dev libsoup2.4-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
	sudo apt install libgtk-3-dev libkeybinder-3.0-dev

Run-time dependencies:

	sudo apt install dconf-gsettings-backend   # to load/store configuration
	sudo apt install gstreamer1.0-plugins-ugly # to listen to mp3 streams
	sudo apt install gstreamer1.0-pulseaudio   # for pulseaudio users
	sudo apt install gstreamer1.0-alsa         # for alsa users

License
-------

Copyright (C) 2015-2017 Arnaud Rebillout <elboulangero@gmail.com>

Goodvibes is released under the [General Public License (GPL) version 3](COPYING).



Translations
------------

Goodvibes uses [Weblate][] to manage translations. If you want to help translating Goodvibes, please visit the project page at <https://hosted.weblate.org/projects/goodvibes>.

[weblate]: https://weblate.org

Hacking
-------

Please refer to the [HACKING](HACKING.md) file to get started.
