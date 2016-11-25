# Git utility functions


GIT_TRACKED=""
GIT_TRACKED_FILLED=0

git_is_tracked()
{
    local file="$1"

    # Fill list of tracked files only once
    if [ $GIT_TRACKED_FILLED -eq 0 ]; then
	GIT_TRACKED="$(git ls-tree -r master --name-only)"
	GIT_TRACKED_FILLED=1
	#echo "--------"
	#echo "$GIT_TRACKED"
	#echo "--------"
    fi

    # Remove leading './' from filename if any
    file="$(echo $file | sed 's:^\.\/::')"
    
    echo "$GIT_TRACKED" | grep -q "$file"
}

git_has_changes()
{
    local file="$1"

    ! git diff-files --quiet "$file"
}
