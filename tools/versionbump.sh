
echo $(awk -v versionDiff="$2" -F. -f "$1/tools/versionbump.awk" OFS=. "$1/.config/version.txt") > "$1/.config/version.txt"

