#!/bin/sh

command -v xdotool >/dev/null 2>&1 || \
	{ echo >&2 "Please install xdotool."; exit 1; }

case $1 in
	play)
		key=XF86AudioPlay
		;;
	pause)
		key=XF86AudioPause
		;;
	stop)
		key=XF86AudioStop
		;;
	prev|previous)
		key=XF86AudioPrev
		;;
	next)
		key=XF86AudioNext
		;;
	*)
		echo >&2 "Usage: $0 <play/pause/stop/prev/next>"
		exit 1
		;;
esac

xdotool key $key

