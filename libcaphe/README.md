Libcaphe
========

Libcaphe is a little library that inhibits sleep/suspend.

You use it in your application if you want to prevent the system from going to
sleep. A typical use-case is for audio/video players, who might want to inhibit
sleep while playing.

The code is inspired by [Caffeine-ng](https://gitlab.com/hobarrera/caffeine-ng).

Libcaphe will work if one of the following D-Bus services is present on your
system:

- org.gnome.SessionManager 
- org.freedesktop.PowerManagement
- org.freedesktop.login1



License
-------

Copyright (C) 2016 Arnaud Rebillout <elboulangero@gmail.com>.

Libcaphe is distributed under the GNU General Public License, either version 3,
or (at your option) any later version. See [LICENSE](LICENSE) for more details.



About the name
--------------

"Cà phê" is the Vietnamese for coffee.

At the moment of this writing, Vietnam is the second largest producer in the
world after Brazil, and I thought the world ought to know :)
