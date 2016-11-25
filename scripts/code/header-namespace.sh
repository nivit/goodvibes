#!/bin/bash -e

source $(dirname $0)/lib-git.sh

print_usage()
{
    echo "Usage: $0 <command> [<file>...]"
    echo
    echo "Commands:"
    echo "  check        Check if the headers have proper ifdef directives"
    echo "  fix          Fix the namespace in the ifdef directives."
    echo "               Assume that ifdef directives are already there,"
    echo "               and that only the namespace needs to be fixed."
}

make_namespace()
{
    local file="$1"
    local bn=$(basename "$file" | tr '\-.' _ | tr '[:lower:]' '[:upper:]')
    local dn=$(dirname "${file##*src}" | sed 's:^/::' | tr '/' '_' | tr '[:lower:]' '[:upper:]')

    echo __OVERCOOKED_${dn}_${bn}__
}

do_check()
{
    local file="$1"
    local ns=$(make_namespace "$file")

    if ! git_is_tracked $file; then
	return 0
    fi

    if [ $(grep $ns "$file" -c) -eq 3 ]; then
	return 0
    else
	return 1
    fi
}

do_fix()
{
    local file="$1"
    local ns=$(make_namespace "$file")

    # Assume the ifdef directives are already there
    sed -i \
	-e "s:#ifndef __.*:#ifndef $ns:" \
	-e "s:#define __.*:#define $ns:" \
	-e "s:#endif /\* __.*:#endif /* $ns */:" \
	$file
}

# Check for help arguments
[ "$1" == "-h" -o "$1" == "--help" ] && \
    { print_usage; exit 0; }

# Check for proper usage
[ $# -lt 1 ] && \
    { print_usage; exit 1; }

# Ensure we're in the right directory
[ -d "src" ] || \
    { echo >&2 "Please run from root directory"; exit 1; }

# File list
if [ $# -eq 1 ]; then
    files=$(find src -name \*.h)
else
    files="${@:2}"
fi

# Do the job
case $1 in
    check)
	for f in $files; do
	    if ! do_check $f; then
		echo "'$f': invalid namespace."
	    fi
	done
	;;

    fix)
	for f in $files; do
	    if do_check $f; then
		continue;
	    fi

	    echo "'$f': fixing namespace."
	    do_fix $f
	done
	;;

    *)
	print_usage
	exit 1
	;;
esac
