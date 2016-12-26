#!/bin/bash

MPRIS_OBJ='org.mpris.MediaPlayer2.Goodvibes'
MPRIS_PATH='/org/mpris/MediaPlayer2'
MPRIS_IFACE='org.mpris.MediaPlayer2'

print_usage()
{
    echo "Usage: $0 <command> [options]"
    echo ""
    echo "Commands:"
    echo "  introspect"
    echo "  call <Root/Player/TrackList> <method> [type:arg] [type:arg] ..."
    echo "  get  <Root/Player/TrackList> <property-name>"
    echo "  set  <Root/Player/TrackList> <property-name> <type:value>"
    echo "  watch-signals"
    echo ""
    echo "Examples:"
    echo "  $0 call Player Play"
    echo "  $0 call TrackList GoTo objpath:\"/org/Goodvibes/StationList/0x55be33e73a60\""
    echo "  $0 call TrackList GetTracksMetadata array:objpath:\"/org/Goodvibes/StationList/0x56502c272620\",\"/org/Goodvibes/StationList/0x56502c272660\""
}

iface_real()
{
    SFX=""

    case $1 in
	Root)
	    SFX="";;
	Player|TrackList)
	    SFX=".$1";;
	*)
	    echo >&2 "Wrong mpris iface"
	    exit 1
    esac

    echo $SFX

}

case $1 in

    introspect)
	dbus-send --print-reply=literal \
		--dest=$MPRIS_OBJ \
	        $MPRIS_PATH \
		org.freedesktop.DBus.Introspectable.Introspect | tr \" \'
		;;

    call)
	IFACE=$2
	METHOD=$3
	dbus-send --print-reply=literal                    \
		  --type=method_call                       \
		  --dest=$MPRIS_OBJ                        \
		  $MPRIS_PATH                              \
		  $MPRIS_IFACE$(iface_real $IFACE).$METHOD \
		  ${@:4}
	;;

    get)
	IFACE=$2
	PROP=$3
	dbus-send --print-reply=literal                     \
       	          --dest=$MPRIS_OBJ                         \
		  $MPRIS_PATH                               \
		  org.freedesktop.DBus.Properties.Get       \
		  string:"$MPRIS_IFACE$(iface_real $IFACE)" \
		  string:"$PROP"
	;;

    set)
	IFACE=$2
	PROP=$3
	TYPEVALUE=$4
	dbus-send --print-reply=literal                     \
       	          --dest=$MPRIS_OBJ                         \
		  $MPRIS_PATH                               \
		  org.freedesktop.DBus.Properties.Set       \
		  string:"$MPRIS_IFACE$(iface_real $IFACE)" \
		  string:"$PROP" variant:$TYPEVALUE
	;;

    watch-signals)
	dbus-monitor "type='signal',sender='$MPRIS_OBJ'"
	;;

    *)
	print_usage
	exit 1
	;;
esac
