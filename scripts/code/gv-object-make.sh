#!/bin/bash -e

type="$1"
objname="$2"
parentname="$3"

print_usage()
{
    echo "Usage: $(basename $0) <type> <object-name> [<parent-name>]"
    echo
    echo "Create a new goodvibes object in './src',"
    echo "that's to say a .c and a .h skeleton file."
    echo
    echo "Parameters:"
    echo "<type>        might be 'core', 'framework', 'ui', 'feat'."
    echo "<object-name> should contain only lowercase, digits and dashes, and start with 'gv-'."
    echo "<parent-name> should contain only lowercase, digits and dashes."
    echo
    echo "Example:"
    echo "    $0 ui gv-about-dialog gtk-about-dialog"
}

name_get_invalid_chars()
{
    # Keep only invalid chars 
    tr -d 'a-z0-9-' <<< "$1"
}

name_to_lower()
{
    # To lowercase, '-' to '_'
    sed -e 's/-/_/g' <<< "$1"
}

name_to_upper()
{
    # To uppercase, '-' to '_'
    sed -e 's/-/_/g' -e 's/\([a-z]\)/\U\1/g' <<< "$1"
}

name_to_camel()
{
    # To camelcase, '-' removed
    sed -e 's/-\([a-z]\)/\U\1/g' -e 's/^\([a-z]\)/\U\1/' <<< "$1"
}



# -------------------------------------------------------- #
# Check and make everything ready                          #
# -------------------------------------------------------- #

# Check for help arguments
[ "$1" == "-h" -o "$1" == "--help" ] && \
    { print_usage; exit 0; }

# Ensure we're in the right directory
[ -d "src" ] || \
    { echo >&2 "Please run from project root directory"; exit 1; }

# Sanity check on <objname>
[ -z "$objname" ] && \
    { print_usage; exit 1; }
grep -q '^gv-' <<< "$objname" || \
    { echo >&2 "'$objname' should start with 'gv-' prefix"; exit 1; }
[ -z "$(name_get_invalid_chars "$objname")" ] || \
    { echo >&2 "'$objname' contains invalid characters"; exit 1; }

# Notice that <objname> has no 'gv-' prefix from now on.
# We explicitely add it when needed.
objname="$(sed 's/^gv-//' <<< $objname)"

# -------------------------------------------------------- #
# Copy skeleton files to source directory                  #
# -------------------------------------------------------- #

# Select source depending on the type
srcdir="scripts/code/gv-object-templates"
srcfile=""
case "$type" in
    core|framework|ui)
	srcfile=gv-dummy;;
    feat)
	srcfile=gv-feature-dummy;;
esac
[ -z "$srcfile" ] && \
    { print_usage; exit 1; }

# Ensure that destination files don't exist
dstdir=""
case "$type" in
    core)
	dstdir="src/core";;
    framework)
	dstdir="src/framework";;
    ui)
	dstdir="src/ui";;
    feat)
	dstdir="src/feat";;
esac
[ -z "$dstdir" ] && \
    { print_usage; exit 1; }

dstfile=gv-$objname
[ -e $dstdir/$dstfile.c ] && { echo >&2 "$dstdir/$dstfile.c already exists"; exit 1; }
[ -e $dstdir/$dstfile.h ] && { echo >&2 "$dstdir/$dstfile.h already exists"; exit 1; }

# Copy files
cp "$srcdir/$srcfile.c" "$dstdir/$dstfile.c"
cp "$srcdir/$srcfile.h" "$dstdir/$dstfile.h"

# -------------------------------------------------------- #
# String substitutions                                     #
# -------------------------------------------------------- #

# Customization for ui files
if [ "$type" == "ui" ]; then
    # Replace 'core' by 'ui' in the include path
    sed -i						\
	-e "s|core/gv-dummy|ui/gv-dummy|"		\
	$dstdir/$dstfile.c
fi

# Customization for framework files
if [ "$type" == "framework" ]; then
    # Replace 'core' by 'framework' in the include path
    sed -i						\
	-e "s|core/gv-dummy|framework/gv-dummy|"	\
	$dstdir/$dstfile.c
fi

# Special customization for ui files
if [ "$type" == "ui" ]; then
    # Add gtk include
    sed -i 						\
	-e "/<glib-object.h>/a #include <gtk/gtk.h>"	\
	$dstdir/$dstfile.c $dstdir/$dstfile.h
    # Return a GtkWidget
    sed -i 				\
	-e "s/^GvDummy \*/GtkWidget */"\
	$dstdir/$dstfile.c $dstdir/$dstfile.h
fi

# Replace 'gobject' and variants by another parent
if [ -n "$parentname" ]; then
    upper="$(name_to_upper $parentname)"
    upper_pfx="$(cut -d_ -f1 <<< $upper)"
    upper_end="$(cut -d_ -f2- <<< $upper)"

    camel="$(name_to_camel $parentname)"
    sed -i 								\
	-e "s/GObject parent_instance/$camel parent_instance/"		\
        -e "/^G_DEFINE/s/G_TYPE_OBJECT/${upper_pfx}_TYPE_${upper_end}/"	\
	$dstdir/$dstfile.c
    sed -i 					\
	-e "/^G_DECLARE/s/GObject/$camel/"	\
	$dstdir/$dstfile.h
fi

# Replace 'dummy' and variants by <objname> 
lower="$(name_to_lower $objname)"
upper="$(name_to_upper $objname)"
camel="$(name_to_camel $objname)"
sed -i					\
    -e "s/gv-dummy/$dstfile/g"		\
    -e "s/gv_dummy/gv_$lower/g"		\
    -e "s/DUMMY/$upper/g"		\
    -e "s/Dummy/$camel/g"		\
    -e "s/dummy/$lower/g"		\
    $dstdir/$dstfile.c $dstdir/$dstfile.h

# Fix things
./scripts/code/copyright.sh add $dstdir/$dstfile.c $dstdir/$dstfile.h
./scripts/code/header-namespace.sh fix $dstdir/$dstfile.h

# Done !
echo "Skeleton files '$dstdir/$dstfile.[ch]' created."
echo "Don't forget to add these new files to './src/Makefile.am'."
