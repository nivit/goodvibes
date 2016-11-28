Coding Style
============



Coding style matters to me. I keep the codebase as clean and neat as possible.
I'm a bit of a maniac about that, I tell you.



Indentation
-----------

The code is currently indented using [Artistic Style](http://astyle.sourceforge.net/).
There's a script to automatically indent the whole thing:

	./scripts/code/indent.sh all

You can also easily indent your staged changes only before commiting:

	./scripts/code/indent.sh staged



Comments & Codetags
-------------------

For comments, always use C style (aka `/* ... */`).

For codetags, always use C++ style (aka `//`).

Here are the codetags in use here:

- `WISHED` Things I wish, but might never be done.
- `TODO`   Things that should be done pretty soon.
- `FIXME`  For things obviously broken.

Always try to make it a one-liner if possible, or at least describe the problem
in one line, then add more details on the following lines (git commit style).

Stick to these conventions, and then getting a TODO list becomes very quick:

	ack 'TODO'

Alternatively, getting the list of things to be done:

	ack '// '
	
Here are some links that discuss codetags:

- <http://legacy.python.org/dev/peps/pep-0350/>
- <http://stackoverflow.com/q/1452934/776208>



GObject C File Layout
---------------------

If you find yourself writing a new file, therefore creating a new object, you
might want to use a script that generates all the boilerplate.

	./scripts/code/ock-object-make.sh

Have a look at the structure of the C file created, and please notice a few
things.

Functions are ordered from low-level to high-level, to avoid the need of
declaring functions before defining it. So basically, you read the file from
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

If you stick to these conventions, then it's easy to find your way in the code:
whatever file you're looking at, they all look the same.
