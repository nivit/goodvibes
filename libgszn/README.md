GObject Serialization (Gszn)
============================



This is a simple library that attempts to provide serialization for GObjects.

When writing it, I came across JSON-GLib at some point. This library provides
an interface for serializing and deserializing GObjects, and was a bit of
inspiration.

- [GObject Serialization](https://developer.gnome.org/json-glib/stable/json-glib-GObject-Serialization.html)
- [Serializable Interface](https://developer.gnome.org/json-glib/stable/json-glib-Serializable-Interface.html)



Overview
--------

Libgszn is a library for serializing GObjects.

The format of the serialization can vary, and at the moment it supports XML
and Keyfile. More backends could be added, but that wouldn't be painless.
The internal of the library is not that clean, and nothing to be proud of.
Some rework would be needed to add, for example, a JSON backend.

The library provides an additional flag for GObject properties:
`GSZN_PARAM_SERIALIZE`, that is used to explicitly indicate that a property
should be serialized.

The __Serializer__ provides a few useful options:
- you can choose to serialize only the flagged properties
- you can choose to serialize only the non-default values

On top of that, the serializer can watch objects for you, and issue a signal
when a change happened.

The __Deserializer__ provides two distinct kind of operations:
- `create()` to create objects from the serialized data.
  A few things must be noted:
  - the types you intend to deserialize must have been registered in
    the GObject type system beforehand. This task is left to you, and you
    should probably issue a few `g_type_class_ref()` calls before deserializing.
  - properties are passed to the object at creation time, so construct-only
    properties are supported.
- `restore()` to configure already existing objects.

Notice that the `restore()` operation suppose that you can match an object
in the serialization data with an existing object. If you serialize only
one instance of each object type, it's trivial. But from the moment you
have more than one instance of a given type, how can you identify which
object in the serialization matches which existing object ?

To solve that problem, the library introduce the notion of __uid__.
You can define an uid on an object by creating a property named
'gszn-uid'. This property MUST be as following:
- Type: string
- Flags: NOT construct-only, NOT serialize

The (de)serializer will handlee this special property automatically,
and then restoring several instances of a same type becomes possible.

The (de)serializer also allows you to hook functions to customize the way
object names and property names are (de)serialized. If you don't provide hooks,
the object type names and the property names are used verbatim.



Keyfile backend limitations
---------------------------

Notice that the keyfile format has a strong limitation: it can't have two
groups with the same name. And since, by default, the object type name is
mapped to the keyfile group, then you can't serialize more than one instance
of a given type.

But thanks to the uid, we can overcome this limitation. When an object has an uid,
it's mapped to the group name, in place of the object type name. The object
type name is then saved to a 'type' key. Which means that 'type' is a reserved
field for keyfile !

So if you serialize to the keyfile format, your objects shouldn't have a 'type'
property. That's it, live with it.

If it's a problem, we could easily use another name though, a one that wouldn't
clash, like 'gszn-type'.

Notice, then, that you're responsible for handling this name clash. The backend
won't help you, and won't detect your mistakes, it's too dumb for that.



Fundamental flaws
-----------------

### String conversion is not what it should be

In the current design, the conversion from a GValue to a string is
performed by the (de)serializer, and he's completely ignorant of
what the backend expects.

It's OK for a backend like XML, which is not typed. So the deserializer
can decide how to trasnform to/from string.

But a format like JSON is kind of typed. For example, to store a boolean,
it expects exactly 'true' or 'false'. Keyfile format also has its
conventions for how to represent a boolean. And the serializer has no
idea about that.

So, I think the backend should provide some transform functions for
some types. The serializer would then use the transform function provided
by the backend if any, or use its own otherwise.



Performance improvments
-----------------------

### Better serialize_print()

At the moment, when serialize_print() is invoked, the serializer
must list all the object properties, and iterate over it to
know which ones to serialize.

I think it wouldn't be too hard to improve that. The serializer
could keep a list of properties to serialize. It would be created
when the objects are added, by scanning all objects and keeping
only the properties we are interested about (aka with serialize flag,
serializable type and so on).

Then, when serialize_print() comes, we could use our internal
properties list instead of requiring again the pspec list,
and iterating on the pspec list.

I'm wondering, however, if it's worth the effort. I'm not sure
the improvment would be very significant. And it wouldn't work
if properties are added to the object at run-time.

Because, it's basically duplicating a subset of the pspec list.
And who duplicates information has to keep it synced. So, maybe
a bad idea.

### Better data structure to exchange parameters with backend

At the moment, the struct GsznParameter is used to exchange the
parameters between the (de)serializer and the backend. This is just
a dumb structure with two strings: key and value. Both strings are
dynamic, and it's a bit of a shame.

When the struct is filled by the serializer, the keys (aka property
names) could be static string.

When the struct is filled by the backend, well, it all depends on the
backend capabilities. The key might be static, or not. Even the value
could be static, why not ?

So, I think we could avoid useless dynamic strings in some case.
Maybe we could just add a flag to GsznParameter to indicate what is
static and what is not (a bit like GValue does to handle strings),
define accessors to ease get/set operations, and then we could take
advantage of static strings when possible.



FAQ
---

### Why does the uid property must be name 'gszn-uid' and type string ?

Well, I had to pick up a name, this one made the most sense.

I tried at some point a smarter implementation, where you can
define, for each object type, the name of the property to be used
as an uid. I also tried to handle it transparently as any property,
so that any type would do.

It's possible to do that but it adds some complexity, and clutters the
API a bit, and also who needs that by the way ?

So, the limitation on this uid property are there for the sake of simplicity.

### How are properties values converted to string ?

Properties are retrieved as GValues. Then we check, given the type of the GValue,
if a transform function exist. If it does, we use it. Otherwise, 
the library takes care of converting, which means that simple types
(numbers) can be converted, more complex types can't.

It means that, if you need to serialize a given type, you should define
a transform function for this type, and register it. That's it.
It's particularly useful for enum types, if you want to serialize with
the name of the enum rather than a number.

### Why having two separate objects, one to deserialize, one to serialize ?

Because otherwise it's a headache. A lot of complexity disappears suddenly
when splitting in two objects, and a lot of problems are solved.

Having two different objects forces to start serializing from scratch,
and that's the main benefit I think.

If you have only one object, you probably won't serializing from scratch,
but instead attempt to update the serialized data. A common scenario is
is that you will deserialize data at startup, then update it as need be
during run-time, and serialize it when closing.

The same data obtained after deserialization is used for serialization.
This creates some problems:

- housekeeping: the lifetime of your serialized data is now longer
  than the runtime of your application. Let's suppose you load some
  serialized data that was serialized with an old version of your
  application, and it contains properties that don't exist anymore.
  Well, you have to clean it up.

  So you need to be able to remove data from the serialization, which
  means a new operation has to be implemented in the backend (there's
  no remove() at the moment, no need for that).

  This problem is automatically solved by starting a new serialization
  from scratch.

- tricky signal catching: let's assume that you use the deserializer
  to restore some objects state. A possible scenario would be that
  you load some serialized data, apply it to a set of objects, and then
  you start watching your object for changes. You watch them only after,
  because you don't want to receive notify signals when you're configuring
  the objects, it looks a bit silly.

  At this moment, you might thing that your serialized data is in sync
  with the objects, since you just configured them yourself. But this
  is not sure. You never know what happens when you set a property.
  Maybe you just have set a property that is deprecated, and that instead
  changed the value of another property. But you didn't catch that since
  you were not wathcing the objects, so you missed some signals.

  If you want to be sure not to miss anything, you must be more smart
  and watch the objects at the same time you configure them. Never tried,
  but the idea sounds rotten and probably leads to other problems.

  Anyway, this problem disappears from the moment you're forced to use
  to different objects for serialization and deserialization.

So, to conclude: having two different objects for serializing and deserializing
definitely forbids you to reuse the deserialized data as the source for the
serialized data, which would be a very bad idea. 

Instead, the only thing you can do with serialized data is to dispose of it
after use. And the only thing you can do if you want to serialize data is to
start from zero, which ensures you have it clean and valid as you expect.

