#!/bin/bash

MODE="$1"
FILES=""
C_FILES=""
H_FILES=""

source $(dirname $0)/lib-git.sh

print_usage() {
    echo "Usage: $0 [<mode>] [<files>]"
    echo 
    echo "Modes:"
    echo "  all          Indent the whole source tree"
    echo "  files        Indent files given in argument"
    echo "  staged       Indent git staged files"
}

do_remove_untracked_files()
{
    FILES_ORIG="$FILES"
    FILES=""

    for file in $FILES_ORIG; do
	if git_is_tracked $file; then
	    FILES="$FILES $file"
	fi
    done
}

do_remove_nonexisting_files()
{
    FILES_ORIG="$FILES"
    FILES=""

    for file in $FILES_ORIG; do
	if [ -f $file ]; then
	    FILES="$FILES $file"
	fi
    done
}

do_split_files()
{
    for file in $FILES; do
	if [[ $file = *.c ]]; then
	    C_FILES="$C_FILES $file"
	elif [[ $file = *.h ]]; then
	    H_FILES="$H_FILES $file"
	fi
    done
}

do_indent()
{
    if [ -z "$FILES" ]; then
	echo "No input files"
	return
    fi

    do_split_files

    echo ">>> Removing trailing whitespaces..."
    sed -i 's/[ \t]*$//' $FILES

    # A few words about options.
    # 'pad-oper' can't be used, since it breaks GQuark definition.
    # For example:
    #     G_DEFINE_QUARK(gszn-error-quark, gszn_error)
    # becomes:
    #     G_DEFINE_QUARK(gszn - error - quark, gszn_error)

    if [ -n "$C_FILES" ]; then
	echo ">>> Indenting code..."
	astyle --suffix=none           \
	       --formatted             \
	       --style=linux           \
	       --indent=tab=8          \
	       --indent-preproc-define \
	       --indent-labels         \
	       --pad-header            \
	       --align-pointer=name    \
	       --convert-tabs          \
	       --max-code-length=100   \
	       $C_FILES
    fi

    if [ -n "$H_FILES" ]; then
	echo ">>> Indenting headers..."
	astyle --suffix=none           \
	       --formatted             \
	       --style=linux           \
	       --indent=tab=8          \
	       --indent-preproc-define \
	       --align-pointer=name    \
	       --convert-tabs          \
	       --max-code-length=100   \
	       --max-instatement-indent=100 \
	       $H_FILES
    fi
}

# Check for proper usage
[ $# -eq 0 ] && \
    { print_usage; exit 0; }

# Check for commands
command -v astyle >/dev/null 2>&1 || \
	{ echo >&2 "Please install astyle."; exit 1; }

# Do the job
case $MODE in
    all)
	[ $# -eq 1 ] || { print_usage; exit 1; }
	FILES="$(find -name '*.[ch]' | tr '\n' ' ')"
	do_remove_untracked_files
	do_indent
	;;

    files)
	FILES="${@:2}"
	do_remove_nonexisting_files
	do_indent
	;;

    staged)
	[ $# -eq 1 ] || { print_usage; exit 1; }
	FILES="$(git diff --name-only --cached | tr '\n' ' ')"
	do_indent
	;;

    *)
	print_usage
	exit 1
	;;
esac
