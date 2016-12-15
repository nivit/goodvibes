#!/bin/sh
# Run this before running ./configure

echo "Running $0..."

autoreconf --force --install --verbose || exit $?

echo "Done, please type './configure' to continue."

