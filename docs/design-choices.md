Design Choices
==============

Before I forget.



Why Implementing The MPRIS2 D-Bus API First ?
---------------------------------------------

Implementing the MPRIS2 D-Bus API was one of the first thing I did. It was not
the easiest thing to start with though, but it forced me to implement all the
common features expected from a music player. Once MPRIS2 was implemented, I
knew I had the right design for the core, and indeed, I never had to rework it
afterwards.

So I really recommend that to anyone writing a music player: implement the
MPRIS2 D-Bus API early, it will help you to get the design right, you won't
regret the pain.



Why Not Choosing Python ?
-------------------------

I definitely wanted to implement the MPRIS2 D-Bus API. Which means implementing
a D-Bus server.

Implementing a fully featured D-Bus server in Python is a bit of a frustrating
experience. Properties are not fully handled. See:

- <https://bugs.freedesktop.org/show_bug.cgi?id=26903>

The quick answer, as I understood it, is that the D-Bus python binding is broken
by design, and there will be no attempt to fix it. This bug will stay forever.

If you have a look at QuodLibet code, you will see that these guys had to write
quite a bit of code to add support for properties. But they did it well, and the
code is easy to take out of QuodLibet. So my guess is that the best way to fully
implement the MPRIS2 D-Bus API is to copy/paste some code from QL.

It's suggested here and there to use another Python D-Bus implementation, like
GLib's implementation GDBus, through PyGObject. However at the moment of this
writing, this is yet to be implemented. See progress here:

- <https://bugzilla.gnome.org/show_bug.cgi?id=656330>

So, with this situation, I decided to go write in C instead, where I was sure
that implementing the MPRIS2 D-Bus API would be satisfactory.

Note that it's probably not an exhaustive view of the situation, and I might
have overlooked other solutions.



Why Choosing GObject ?
----------------------

GObject has its cons, that I was very aware of when I started to learn. As often
when I face a new technology, I was a bit reluctant and only saw the downside:

- Boilerplate: there so much copy/paste to do that I ended up writing a script
to automate C/H file creation.
- Magic: some of the boilerplate is hidden by macros. It's great since it saves
code typing, but it's bad since it makes some definitions implicit. So you end
up calling functions or macros that are defined nowhere, and it's a bit
unsettling at first. GObject code looks like dark magic for the newcomer.
- Steep learning curve: yep, learning GObject by yourself can be a bit tedious.

On the other hand, GObject brings in everything I needed to implement cleanly
this MPRIS2 D-Bus API.

- signals: from the moment I have signals internally, there's no need to hack
hack around to send signals on D-Bus. It's trivial.
- properties: same thing. From the moment the code is GObject-oriented, getting
and setting properties through D-Bus is trivial.
- memory management: it's handled internally by GObject, nice and clean.

Plus, in some situation you realise that GObject actually saves quite some code
typing, because things happen automatically. For example, `g_object_bind_property()`,
heavily used to map UI widgets to internal objects property, is a real time-saver,
and it's lovely to use.

Using GObject is more than just using a library. It really decides of the
architecture of your application. It becomes event-driven code (event though
events are called signals here), not really the kind of programming we're used
to in C.



Why Not Using `gdbus-codegen` ?
-------------------------------

First, I'm not a big fan of code generation.

Second, as far as I remember, the generated code produces some warnings at
compile time, and I usually compile with the flag `-WError`. It's annoying.

Third, there's a bug about `Property.EmitsChangedSignal`, that has been pending
forever, and in all likelihood will never be fixed.

- <https://bugzilla.gnome.org/show_bug.cgi?id=674913>

This bug affects the MPRIS2 D-Bus API, so I prefer to stay away from that
`gdbus-codegen` and remain in control of the code.



Why Using Explicit Notify Only ?
--------------------------------

This is just an convention in use in the code, I just find it more logical when
I started, and I never had to come back on this choice.

For this reason, every property is created with the `G_PARAM_EXPLICIT_NOTIFY`
flag, and emitted only if the value of the property changed. It's a bit more
job on the object where the properties are changed, but it's less job on the
object listening for signals, since they can be sure that they have to do
something when the signal handler is invoked.



Why Having This Feature Thing ?
-------------------------------

You will see in the code that some features are quite isolated from the rest
of the code. First, there's a `GvFeature` object, that is the parent of all
features. Then, features just need to inherit this object, connect to some
signals from the core, and that's it.

Isolating features is simply a way to keeps things cleanly separated, and to
allow to disable some code at compile time, without having to cripple the code
with `#ifdef`. It's possible thanks to the fact that features just react to
signals. The core of the code itself is blissfully ignorant of the existence
of the features, and never explictly invokes it.

Features are created once at init, destroyed once at cleanup, and that's the
only place where you will see `#ifdef`. They live in a world of their own.

Such a strong isolation has some benefits: easier debug and maintenance,
mainly. And it's nice to be able to disable it at compile-time, if ever people
want to integrate the application in an environment where some of these
features don't make sense (like on embedded devices).

It also forces a better overall design. However, it can be a pain in the ass
when a new feature shows up, and doesn't quite fit in, and the whole thing
needs to be reworked, and all the existing features need to be updated to match
the changes...

Ultimately, with some improvments, these features could be loaded dynamically,
and from this point they could be called plugins. And then, it's very common
to have plugins in GNU/Linux media player. So, this features are like a poor's
man plugin.



Why Using GtkStatusIcon To Display The Tray Icon ?
--------------------------------------------------

As everybody know, the tray icon situation on GNU/Linux at the moment is a bit
problematic.

- `GtkStatusIcon` was the obvious choice for a long time (for GTK+ programs), but
it's been deprecated in GTK+3, and removed in GTK+4.
- `LibAppIndicator` was born in an attempt to solve the problem. It's originally
from Ubuntu, and is also now KDE's choice. Apparently, the status icon is
handled through a D-Bus API. If the D-Bus service is not found, the library
falls back to GtkStatusIcon.
- I don't know about `Gnome`, but there was a discussion at a time, where in the
end they refused to adopt libappindicator, and instead went for implementing
their own solution.
- `libsystemtray`: I don't know who uses that.

Ok, so, I personally uses OpenBox/Tint2. These guys are the looser in the story,
they relied on GtkStatusIcon, what future for them ? It's unsure that they have
the resource needed to develop a new solution, or to go for libappindicator.

Anyway, since I had some experience using GtkStatusIcon, I chose to use it to
implement the tray icon. Ultimately, I'd like to be able to implement different
tray icon backend, to support Gnome and libappindicator. But for the moment,
I'll stick to this good old GtkStatusIcon.

Not a good choice, but maybe there's no good choice at the moment.



Station List: Shuffle Implementation
------------------------------------

Things I've tried and didn't work.

One idea was that the player would have two StationList objects. One would be
ordered, the other would be shuffled. With that, the 'shuffle problem' is taken
out of the StationList object, which is therefore esier to implement, and is
then solved outside of it. Mainly in the player, I thought.

In practice, it adds quite a lot of complexity, because these two lists must be
kept in sync (from a more genereal point of view, from the moment you duplicate
data, you need to keep it in sync, and that's why duplicating data is always
something to avoid). Furthermore, the station list is a global variable, that
can be accessed by anyone. How to handle duplication then ?

We could have both list (ordered and shuffled) global. But it would be a mess
for the rest of the code. Always wondering which one you must deal with, and
what if you want to add a new station ? So the code is responsible for adding it
to both list ? And therefore, the code is responsible for keeping lists in sync ?

Of course not, the sync between both lists must be automatic, but if both are
global, it means that they both must watch each other's signals to keep in sync.
Or another object must watch both of them and keep them in sync. This doesn't
sound sweet to my ears.

Another possibility is to have only the ordered station list public, and the
shuffled one private to the player, and then the player would be left with the
responsability to keep the shuffled list in sync, somehow. Hmmm, I don't like
that too much either.

OK, so let's forget about that. If the shuffle problem is not to be solved
outside of the list, then let's solve it within the list. We could have a
'shuffle' property added to the station list, and that's all. The code set it
to true or false, and doesn't bother anymore.

It kind of works, but the StationList API becomes confusing. Because if you look
at it as it is, you will notice that some functions are expected to support the
shuffle property (obviously, next() and prev() will return a different value
depending on the shuffle property), while other would actually not care about
the shuffle property (append(), prepend() would only deal with the ordered list).

So, completely hiding the 'shuffle' from the outside world is also not a very
good solutions. At some point, when you use the API, you start to find it
confusing. Even if you set the shuffle property, some functions keep on dealing
with an ordered list internally. It's magic, and we don't like magic.

Actually, from the user point of view, if you enable shuffle, the list still
appears ordered in the ui. It just affects the previous and next actions. And
the best is to see this behavior appearing in the API.

So in the end, the best implementation of the shuffle I came with is a 50/50:
both the Player object and the StationList object know about it, and both do
their job handling it. It appears explicitely in the StationList API, and only
makes sense for next() and prev(). The Player is the 'owner' of the shuffle
setting, and feeds it to StationList when calling next() and prev().

In the end, this implementation is the most simple, and leaves little place for
magic. I've been happy with it for a while now.
