
version=$(cat "$1/.config/version.txt")

echo $(awk -v versionDiff="$2" -F. -f "$1/tools/versionbump.awk" OFS=. <<< "$version") > "$1/.config/version.txt"

