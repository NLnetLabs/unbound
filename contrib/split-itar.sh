#/usr/bin/env bash
# Contributed by Tom Hendrikx <tom@whyscream.net>

PROGNAME=$(basename $0)

usage() {
	echo "$PROGNAME: split the anchors.mf file from IANA into separate files." >&2
	echo "" >&2
	echo "$PROGNAME uses 2 arguments:" >&2
	echo "  - the path to the anchors.mf, available at: https://itar.iana.org/" >&2
	echo "  - the directory to leave the files, defaults to '.' (current working dir)" >&2
	exit 1
}

if [ -n "$1" ] && [ -r "$1" ]; then
	itar="$1"
	echo "Reading from: $itar"
else 
	echo "Error: no anchors file given" >&2
	usage
fi

if [ -n "$2" ]; then
	dir="$2"
else 
	dir=$(pwd)
fi

if [ ! -d "$dir" ]; then
	echo "Error: $dir is not a directory" >&2
	usage
fi

while read cn line; do
	if [ $(expr match "$cn" '[a-zA-Z0-9-]*\.') -gt 0 ]; then
		# first line of key
		out="$dir/$cn"anchor
		echo "writing key for $cn to: $out"
		echo "$cn	$line" > $out
	elif [ "$cn" == "DS" ]; then
		# second or later line of earlier defined key
		echo "	$cn	$line" >> $out
	fi
done < "$itar"

echo "Done."
