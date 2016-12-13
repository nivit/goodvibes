#!/bin/sh
# https://www.sourceware.org/autobook/autobook/autobook_43.html

echo "Running $0..."

autoreconf --force --install && \
intltoolize --copy --force --automake

echo "Done, please type './configure' to continue."

