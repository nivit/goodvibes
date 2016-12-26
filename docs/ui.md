User Interface
==============



How much margin and spacing for the UI ?
----------------------------------------

When designing the UI in Glade, there's no margins by default, so the whole
thing looks ugly at this point. Margins, spacings, paddings and such must be
set, and it's up to me to decide how much. So I wonder: how much ?

I have no idea, and don't especially care, so I let someone else decide for me.
I just followed the guidelines provided by GNOME.

- [GNOME HUman Interface Guidelines](https://developer.gnome.org/hig/stable/)



Why not using GtkDialog to handle preferences ?
-----------------------------------------------

Mainly, it's because a dialog requires a transient parent, aka another window
that must be the parent. Ever had this warning in your console ?

	Gtk-Message: GtkDialog mapped without a transient parent. This is discouraged.

But Goodvibes is mainly a status icon, and doesn't really have a main window to
speak of.

Loosely speaking, we could use the popup window as the transient parent, since
it always exists. It's hidden most of the time, but it exists. But it looks a
bit hacky to me. I don't really know why GTK+ needs a transient parent, and what
it expects from this parent, but I'm almost sure the popup window is not up to
the expectation.

So in the end, I find it cleaner to use a window rather than a dialog.



Should I use the GObjet property details in the UI ?
----------------------------------------------------

Short answer: No

It might be tempting though, simply to avoid duplicating strings definition.

Indeed, in the preferences dialog, often a setting is mapped to an object 
property. Given that a property already has a nick (that could be mapped to the
setting label) and a blurb (aka a description that could be mapped to the
setting tooltip), why should we define these strings again in the glade file ?
Why don't we just set labels and tooltips from the property nick and blurb in
the C code ?

It's tempting. Even more after reading this piece of the doc:

- <https://developer.gnome.org/gobject/stable/gobject-GParamSpec.html#g-param-spec-internal>

However, I have good reasons not to do so, both for the nicks and blurbs
(although in the following, I'll only talk about nicks, for clarity).

First, there's the context. Let's suppose you have a property that decides what
action should be associated with a middle-click. In your code, an ideal nick
for the property would be "Mouse Middle Click". However, assuming that in the
UI you have a "Mouse" section for the settings related to the mouse, then you
don't want to repeat the word "Mouse". The ideal name for the setting would
then be "Middle Click".

So, because of a different context, the best name for a property in the code,
and the best name for the corresponding setting in the UI, might be different.

Second, there's more problems when translation kicks in. Because then, the
property nicks must be translatable, and if I want to be consistent, then
__ALL__ the properties should have a translatable nick. It means that those
properties which don't appear in the UI will be translated for nothing. Useless
work for translators, isnt't it ? Or should I only translates the nicks that
appear in the UI ? But then how do I maintain this mess ? What will happen, in
the end, is that I'll have nicks marked as translatable, but don't appear in
the UI, while others appear in the UI but are not marked as translatable...

The REAL problem, here, is that there will be no warnings for these missing
translations. So there will always be missing translations in each new release.

So, here are my two main issues: context, and translations.

In the end, my choice is to use nicks and blurbs only as hints for developpers
passing by. For the UI, I'll define everything in Glade, or in the related C
file. It solves the context issue, and also the translation issue, since Glade
automatically mark strings as translatable, so I don't have to bother.

More interesting reading:

- <https://mail.gnome.org/archives/desktop-devel-list/2014-February/msg00154.html>
