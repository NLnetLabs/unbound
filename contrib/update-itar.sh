#!/bin/sh
# update-itar.sh - update from the interim trust anchor repository
# Copyright 2009, W.C.A. Wijngaards
# This file is BSD licensed, see doc/LICENSE.

# --- Some settings

# directory where unbound works
thedir="."
# where is the file that unbound is going to read
ub_ta_file="$thedir/anchors.mf"
# where is the itar master file format
itar_url="ftp://ftp.iana.org/itar/anchors.mf"
# where is the itar PGP signature
itar_sig="ftp://ftp.iana.org/itar/anchors.mf.sig"

# which command to fetch urls, cmd $dest $url. "wget -O" "curl -o"
fetch_cmd="wget -O"
# file with pgp public key
pgp_pub_key_file="$thedir/update-itar.key"
# our pgp keyring (goes into .gnupg directory)
pgp_keyring_file="update-itar.ring"
# pgp command to use
pgp_cmd="gpg"


# --- The script is below
usage ( )
{
	echo "usage: update-itar"
	echo "    Updates the trust anchors from the interim trust"
	echo "    anchor repository, https://itar.iana.org, and checks PGP sig."
	echo
	echo "    Updates $ub_ta_file with the latest keys."
	echo "    Read that file from the unbound config with"
	echo "    trust-anchor-file: "'"'"$ub_ta_file"'"'
	echo
	echo "    Exit code 0 means anchors updated, 1 no changes, "
	echo "    others are errors. So, in a cronjob you can do:"
	echo "    cd /usr/local/etc/unbound    # your unbound work dir"
	echo "    ./update-itar.sh && unbound-control reload"
	exit 2
}

if test $# -ne 0; then
	usage
fi
tmpf="/tmp/update-itar.$$"

# one argument: explanation string
error_exit ( )
{
	if test -f $tmpf.log; then cat $tmpf.log; fi
	rm -f $tmpf $tmpf.sig $tmpf.log
	echo "Error updating trust anchors: $1"
	exit 2
}

if test ! -f $pgp_pub_key_file || test ! -f $HOME/.gnupg/$pgp_keyring_file || \
	test "$pgp_pub_key_file" -nt $HOME/.gnupg/$pgp_keyring_file; then
	# default key contents right here
	if test ! -f $pgp_pub_key_file; then
		echo "creating default IANA ITAR pgp key file"
		cat >$pgp_pub_key_file <<EOF
-----BEGIN PGP PUBLIC KEY BLOCK-----
Version: GnuPG v1.4.5

mQGiBElr2DcRBAC+6YK6eSP7rzstvnMPQXMrpvVfuIR5FeTpGuwae9JP78V/iOXr
N0yW8Dn6kdAztCMuRizL1Ij9IgaD7pjn8h09VgR4cN4LDv75rcQeWLzNxKy4UNRF
aStL77RcIoTblBeCgHAK9FLzd0XfTGZCNaLNy9BYVSLyADOVDIqgBcrvBwCglz03
QhOMIgaSx/XuRh6kYtynZ6kD/2GXx6pFs57b7rww8yOpdurCSOMB1wuEXiIXznTI
06ARiib0G5VDvOdpy0LDU2526Q9f/WAERlhcExTgnTFigG4mRksUiDrrai4GIr+6
JaivcGFVYdZZ4mZ088jcwujS/UY3C0ryGR9ufYUDAnfx6frhSl6o6j5is+jeGndF
JYRAA/9B/1OXNVwFSiIxnP2aPUwsT1li1vaW8dhA/5PcuPLOVvEjPc1Pc16HGLhE
8CRmMn66LqB1ccInE5hLKGGvV3pctjan+IOhaq3OHt/a+buDtTPgykchMZ2k1AzT
RYk+gksxpIl6yTZsBH4hoRt8auxEJW8AiYbNtXXkNuWcoQL40bQsSUFOQSBUcnVz
dCBBbmNob3IgUmVwb3NpdG9yeSA8aXRhckBpYW5hLm9yZz6IYAQTEQIAIAUCSWvY
NwIbAwYLCQgHAwIEFQIIAwQWAgMBAh4BAheAAAoJEPR9+zCB1GT0GUAAn29/TacF
Teh87dls8pmkjxS4pKf1AKCJS/MvzR54AblO4DNMyc9q0G4frrkCDQRJa9g3EAgA
ywoLxF4HVb9o926UIXw8JxNIzDPkt8galAcKRUmHQMNa/QA80WMl9Ia6DIxavGlV
I5O1fvms297RV2KSSBjKWn6G+0me80A7aw0eHNg7habM5VtzDz5DhJbQFdJV9aYV
DoTSnY6uR6iSSRrdZNaYdlCwDS8lBCnOKoGMArHzVCa2EdCBeCUY/eObOXtu8Gm3
nDkuWeLPv08/0lvtr6d6VoDUEVPEsJAPONEYtpz/D+EZddUraF+3JscWqfRApBOz
/8WHaeTNdzIH+h1ntktiizA6eH40EM6coQQxtIRsxg1DPTxofdovreMkcMI0EUUP
awDn8gDtgG3g6Ud5zCdknwAEDQf/W3rxrEN6OZxJvWti8Iu6KOgxtuINiAsimPPX
qy9KHowyEE9EMPzgADjWC9Reyusr4CwcootjWw7ryUwU0fXvVULKhg32NzLsx/Ci
WtgCPSR58WZ1KKhnoB05+PTrwzhU+u64Cd/vJtFqGxSrANl2FAkPswHJMr8dMwAz
uni7zzLJ+homW1T5AaixwmN3jeDHWshJH9E9JIhr5Y/8AzMl1V10r2u1c2ej0lBJ
Y4GegI5cYAEBerS9d+mrbPlvbJ8AfuAuEf0y/PWJh0z1+Kck5qIbXMn/rpMBYvLJ
Uj5CfqWlh8+hxqSYJDXtLm8hBiQFiMEew0jOc2Tw4F91OZ+jyYhJBBgRAgAJBQJJ
a9g3AhsMAAoJEPR9+zCB1GT0AUwAn2ZtBwAyVxppdeTqilXufUvAkvjbAJ9dUpR1
9a17/5BvDDJcIxSEKTJmDw==
=zCNZ
-----END PGP PUBLIC KEY BLOCK-----
EOF
	fi
	# import the new key
	$pgp_cmd --no-default-keyring --keyring $pgp_keyring_file \
		--primary-keyring $pgp_keyring_file \
		--import $pgp_pub_key_file >$tmpf.log 2>&1 \
		|| error_exit "could not import pgp public key into keyring"
fi

$fetch_cmd $tmpf $itar_url >$tmpf.log 2>&1 \
	|| error_exit "fetching $itar_url failed"
tail -2 $tmpf | grep "; End of file" >/dev/null 2>&1 || \
	error_exit "The file fetched from $itar_url was partial"
$fetch_cmd $tmpf.sig $itar_sig >$tmpf.log 2>&1 \
	|| error_exit "fetching $itar_sig failed"

# check the file with pgp
$pgp_cmd --no-default-keyring --keyring $pgp_keyring_file \
	--verify $tmpf.sig $tmpf >$tmpf.log 2>&1 \
	|| error_exit "the PGP signature failed!"

# check for differences
val=1
if diff "$ub_ta_file" $tmpf 2>/dev/null ; then
	# echo "The interim trust anchor repository did not change."
	:
else
	echo "Updating $ub_ta_file"
	cp $tmpf $ub_ta_file
	val=0
fi

rm -f $tmpf $tmpf.sig $tmpf.log
exit $val 
