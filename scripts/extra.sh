#!/bin/bash

ACTION="$1"

BASE_URL=''
BASE_URL_GIT='git@github.com:elboulangero'
BASE_URL_HTTPS='https://github.com/elboulangero'

DEBIAN_REPO='goodvibes-debian.git'
DOCKER_REPO='goodvibes-docker.git'
WIKI_REPO='goodvibes.wiki.git'

print_usage()
{
    echo "Usage: $0 [<action>]"
    echo 
    echo "Actions:"
    echo "  clone        Clone extra repositories"
    echo "  update       Update extra repositories"
}

clone_repo()
{
    local url="$1"
    local dir="$2"

    if [ -d "$dir" ]; then
	echo "'$dir' already exists !"
	return
    fi

    git clone "$url" "$dir"
}

update_repo()
{
    local dir="$1"

    if ! [ -d "$dir" ]; then
	echo >&2 "'$dir' is not a directory !"
	return
    fi

    # Execute in a sub-shell because we change directory
    (
	cd $dir

	if ! [ -d ".git" ]; then
	    echo >&2 "'$dir' is not a git repository !"
	    exit
	fi

	if [ -n "$(git status --porcelain)" ]; then
	    echo >&2 "'$dir' is not clean !"
	    exit
	fi

	git pull --rebase
    )
}

do_clone()
{
    echo ">>> $BASE_URL/$DEBIAN_REPO"
    clone_repo $BASE_URL/$DEBIAN_REPO extra/debian

    echo ">>> $BASE_URL/$DOCKER_REPO"
    clone_repo $BASE_URL/$DOCKER_REPO extra/docker

    echo ">>> $BASE_URL/$WIKI_REPO"
    clone_repo $BASE_URL/$WIKI_REPO extra/wiki    
}

do_update()
{
    echo ">>> $BASE_URL/$DEBIAN_REPO"
    update_repo extra/debian

    echo ">>> $BASE_URL/$DOCKER_REPO"
    update_repo extra/docker

    echo ">>> $BASE_URL/$WIKI_REPO"
    update_repo extra/wiki
}

# Check for help arguments
[ "$1" == "-h" -o "$1" == "--help" ] && \
    { print_usage; exit 0; }

# Check for proper usage
[ $# -ne 1 ] && \
    { print_usage; exit 0; }

# Ensure we're in the right directory
[ -d "extra" ] || \
    { echo >&2 "Please run from root directory"; exit 1; }

# Get base url
url="$(git config --local --get remote.origin.url)"
if [ "$(echo ${url:0:3})" = "git" ]; then
    BASE_URL="$BASE_URL_GIT"
elif [ "$(echo ${url:0:5})" = "https" ]; then
    BASE_URL="$BASE_URL_HTTPS"
else
    echo >&2 "Unrecognized url for remote origin"
    exit 1
fi

# Do the job
case $ACTION in
    clone)
	do_clone
	;;

    update)
	do_update
	;;

    *)
	print_usage
	exit 1
	;;
esac
