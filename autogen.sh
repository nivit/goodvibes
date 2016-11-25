#!/bin/sh
# https://www.sourceware.org/autobook/autobook/autobook_43.html

echo "Running $0..."

aclocal \
&& automake --foreign --add-missing \
&& autoconf

echo "Done, please type './configure' to continue."

