#!/bin/sh
# update-anchor.sh, update a trust anchor.
# this file is BSD licensed.

# which validating lookup to use.
ubhost=unbound-host

usage ( )
{
	echo "usage: update-anchor [-b] <zone name> <trust anchor file>"
	echo "    performs an update of trust anchor file"
	echo "    the trust anchor file is overwritten with the latest keys"
	echo "    the trust anchor file should contain only keys for one zone"
	echo "    -b causes keyfile to be made in bind format."
	echo "       without -b the file is made in unbound format."
	echo "    "
	echo "alternate:"
	echo "    update-anchor [-b] -d directory"
	echo "    update all <zone>.anchor files in the directory."
	echo "    "
	echo "    name the files br.anchor se.anchor ..., and include them in"
	echo "    the validating resolver config file."
	echo "    put keys for the root in a file with the name root.anchor."
	exit 1
}

if test $# -eq 0; then
	usage
fi
bindformat="no"
filearg='-f'
if test X"$1" = "X-b"; then
	shift
	bindformat="yes"
	filearg='-F'
fi
if test $# -ne 2; then
	echo "arguments wrong."
	usage
fi

do_update ( ) {
	# arguments: <zonename> <keyfile>
	zonename="$1"
	keyfile="$2"

	tmpfile="/tmp/update-anchor.$$"
	$ubhost -v $filearg "$keyfile" -t DNSKEY "$zonename" >$tmpfile
	if test $? -ne 0; then
		rm -f $tmpfile
		echo "Error: Could not update zone $zonename anchor file $keyfile"
		echo "Cause: $ubhost lookup failed" 
		echo "    (Is the domain decommissioned? Is connectivity lost?)"
		return 2
	fi

	# has the lookup been DNSSEC validated?
	if grep '(secure)$' $tmpfile >/dev/null 2>&1; then
		:
	else
		rm -f $tmpfile
		echo "Error: Could not update zone $zonename anchor file $keyfile"
		echo "Cause: result of lookup was not secure" 
		echo "    (keys too far out of date? domain changed ownership?)"
		return 3
	fi

	if test $bindformat = "yes"; then
		# are there any KSK keys on board?
		echo 'trusted-keys {' > "$keyfile"
		if grep ' has DNSKEY record 257' $tmpfile >/dev/null 2>&1; then
			# store KSK keys in anchor file
			grep '(secure)$' $tmpfile | \
			grep ' has DNSKEY record 257' | \
			sed -e 's/ (secure)$/";/' | \
			sed -e 's/ has DNSKEY record \([0-9]*\) \([0-9]*\) \([0-9]*\) /. \1 \2 \3 "/' | \
			sed -e 's/^\.\././' >> "$keyfile"
		else
			# store all keys in the anchor file
			grep '(secure)$' $tmpfile | \
			sed -e 's/ (secure)$/";/' | \
			sed -e 's/ has DNSKEY record \([0-9]*\) \([0-9]*\) \([0-9]*\) /. \1 \2 \3 "/' | \
			sed -e 's/^\.\././' >> "$keyfile"
		fi
		echo '};' >> "$keyfile"
	else #not bindformat
		# are there any KSK keys on board?
		if grep ' has DNSKEY record 257' $tmpfile >/dev/null 2>&1; then
			# store KSK keys in anchor file
			grep '(secure)$' $tmpfile | \
			grep ' has DNSKEY record 257' | \
			sed -e 's/ (secure)$//' | \
			sed -e 's/ has DNSKEY record /. IN DNSKEY /' | \
			sed -e 's/^\.\././' > "$keyfile"
		else
			# store all keys in the anchor file
			grep '(secure)$' $tmpfile | \
			sed -e 's/ (secure)$//' | \
			sed -e 's/ has DNSKEY record /. IN DNSKEY /' | \
			sed -e 's/^\.\././' > "$keyfile"
		fi
	fi # endif-bindformat

	echo "$zonename key file $keyfile updated."

	rm -f $tmpfile
}

if test X"$1" = "X-d"; then
	tdir="$2"
	echo "start updating in $2"
	for x in $tdir/*.anchor; do
		if test `basename "$x"` = "root.anchor"; then
			zname="."
		else
			zname=`basename "$x" .anchor`
		fi
		do_update "$zname" "$x"
	done
	echo "done updating in $2"
else
	# regular invocation
	if test X"$1" = "X."; then
		zname="$1"
	else
		# strip trailing dot from zone name
		zname="`echo $1 | sed -e 's/\.$//'`"
	fi
	kfile="$2"
	do_update $zname $kfile
	exit $?
fi

exit 0
