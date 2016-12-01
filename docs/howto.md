How To
======



Add a setting to the configuration file
---------------------------------------

Configuration is done through GObject serialization. Every setting in the
configuration file is mapped to a GObject property.

If you need to add a new setting to the configuration, the procedure is as
follow:

- Add a property to your object.
- Flag it with the `SERIALIZABLE` flag.
- Ensure the object is added to the list of configurable objects.

That's all. The property will be automatically saved when it's modified, and
restored when the application is started up.

Please notice that it works only for simple types that can be automatically
converted to string. For more complex situations, you might need to get your
hands dirty and dive into `libgszn`, in charge of serializing the GObjects.



Report an error to the user
---------------------------

In order to report an error to the user, just follow the following steps:

- Have your object implement the `errorable` interface.
- Emit an error signal with `ock_errorable_emit_error()`.
- Ensure the object is added to the list of errorable objects.
