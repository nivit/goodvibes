#!/bin/bash

# Ensure required environment variables are defined
[ -z "$DEBEMAIL" ]    && { echo >&2 "DEBEMAIL var is not defined !"; exit 1; }
[ -z "$DEBFULLNAME" ] && { echo >&2 "DEBFULLNAME var is not defined !"; exit 1; }

# Get current version
TAG="$(git tag --points-at HEAD)"
[ -z "$TAG" ] && { echo >&2 "No tag at current revision !"; exit 1; }

# Check if an archive already exists
ARCHIVE="goodvibes-$TAG.tar.gz"
[ -f "$ARCHIVE" ] || { echo >&2 "'$ARCHIVE' does not exist !"; exit 1; }

# Check that the debian files are here as well
DEB=extra/debian
[ -d "$DEB" ] || { echo >&2 "'$DEB' does not exist !"; exit 1; }

# Check that packaging directory doesn't exist yet
PKGDIR=".packaging"
[ -d "$PKGDIR" ] && { echo >&2 "'$PKGDIR' already exists !"; exit 1; }

# Do the job
mkdir "$PKGDIR"
cp "$ARCHIVE" "$PKGDIR"
cd "$PKGDIR"
tar -xf "$ARCHIVE"
cp -r ../$DEB goodvibes-$TAG
cd goodvibes-$TAG

# Create the hierarchy as expected
echo ">>>> Creating hierarchy"
dh_make --single --copyright gpl3 -f ../$ARCHIVE

# Update the changelog
echo ">>>> Updating changelog"
debchange -v $TAG-1

# Tweak the changelog for ubuntu
sed -i 's/unstable;/xenial;/g' debian/changelog
sed -i 's/UNRELEASED;/xenial;/g' debian/changelog

# Build the source
echo ">>>> Building the source package"
debuild -S -sa

# Build the binary
#echo ">>>> Building the binary package"
#dpkg-buildpackage -us -uc

# Done ! Time to upload
echo "Done ! Time to upload ! Please run that !"
echo "  dput goodvibes-ppa $PKGDIR/goodvibes_$TAG-1_source.changes"
