#!/bin/sh

# Build unbound distribution tar from the SVN repository.
# 
# Copyright (c) 2007, NLnet Labs. All rights reserved.
# 
# This software is open source.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
# 
# Neither the name of the NLNET LABS nor the names of its contributors may
# be used to endorse or promote products derived from this software without
# specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# Abort script on unexpected errors.
set -e

# Remember the current working directory.
cwd=`pwd`

# Utility functions.
usage () {
    cat >&2 <<EOF
Usage $0: [-h] [-s] [-d SVN_root] [-l ldns_path]
Generate a distribution tar file for NSD.

    -h           This usage information.
    -s           Build a snapshot distribution file.  The current date is
                 automatically appended to the current NSD version number.
    -d SVN_root  Retrieve the NSD source from the specified repository.
                 Detected from svn working copy if not specified.
    -l ldnsdir   Directory where ldns resides. Detected from Makefile.
EOF
    exit 1
}

info () {
    echo "$0: info: $1"
}

error () {
    echo "$0: error: $1" >&2
    exit 1
}

question () {
    printf "%s (y/n) " "$*"
    read answer
    case "$answer" in
        [Yy]|[Yy][Ee][Ss])
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

# Only use cleanup and error_cleanup after generating the temporary
# working directory.
cleanup () {
    info "Deleting temporary working directory."
    cd $cwd && rm -rf $temp_dir
}

error_cleanup () {
    echo "$0: error: $1" >&2
    cleanup
    exit 1
}

replace_text () {
    (cp "$1" "$1".orig && \
        sed -e "s/$2/$3/g" < "$1".orig > "$1" && \
        rm "$1".orig) || error_cleanup "Replacement for $1 failed."
}

replace_all () {
    info "Updating '$1' with the version number."
    replace_text "$1" "@version@" "$version"
    info "Updating '$1' with today's date."
    replace_text "$1" "@date@" "`date +'%b %e, %Y'`"
}
    

SNAPSHOT="no"
LDNSDIR=""

# Parse the command line arguments.
while [ "$1" ]; do
    case "$1" in
        "-h")
            usage
            ;;
        "-d")
            SVNROOT="$2"
            shift
            ;;
        "-s")
            SNAPSHOT="yes"
            ;;
        "-l")
            LDNSDIR="$2"
            shift
            ;;
        *)
            error "Unrecognized argument -- $1"
            ;;
    esac
    shift
done

# Check if SVNROOT is specified.
if [ -z "$SVNROOT" ]; then
    if test -f .svn/entries; then
	  eval `svn info | grep 'URL:' | sed -e 's/URL: /url=/' | head -1`
	  SVNROOT="$url"
    fi
    if test -z "$SVNROOT"; then
	error "SVNROOT must be specified (using -d)"
    fi
fi
# Check if LDNSDIR is specified.
if test -z "$LDNSDIR"; then
    # try to autodetect from Makefile (if present)
    if test -f Makefile; then
	  eval `grep 'ldnsdir=' Makefile`
	  if echo "$ldnsdir" | grep -v ldns-src/ >/dev/null 2>&1; then
	  	LDNSDIR="$ldnsdir"
	  fi
    fi
fi

# Start the packaging process.
info "SVNROOT  is $SVNROOT"
info "SNAPSHOT is $SNAPSHOT"

#question "Do you wish to continue with these settings?" || error "User abort."


# Creating temp directory
info "Creating temporary working directory"
temp_dir=`mktemp -d unbound-dist-XXXXXX`
info "Directory '$temp_dir' created."
cd $temp_dir

info "Exporting source from SVN."
svn export "$SVNROOT" unbound || error_cleanup "SVN command failed"

cd unbound || error_cleanup "Unbound not exported correctly from SVN"

info "Adding libtool utils (libtoolize)."
libtoolize -c || error_cleanup "libtoolize failed"

info "Building configure script (autoreconf)."
autoreconf || error_cleanup "Autoconf failed."

rm -r autom4te* || error_cleanup "Failed to remove autoconf cache directory."

info "Building lexer and parser."
echo "#include \"util/configyyrename.h\"" > util/configlexer.c || error_cleanup "Failed to create configlexer"
flex -i -t util/configlexer.lex >> util/configlexer.c  || error_cleanup "Failed to create configlexer"
bison -y -d -o util/configparser.c util/configparser.y || error_cleanup "Failed to create configparser"

# check shared code, ldns-testpkts from ldns examples, if possible.
cd ../..
if test ! -z "$LDNSDIR"; then
	if diff -q $LDNSDIR/examples/ldns-testpkts.c testcode/ldns-testpkts.c &&
	   diff -q $LDNSDIR/examples/ldns-testpkts.h testcode/ldns-testpkts.h; then
	   	info "ldns-testpkts.c and ldns-testpkts.h are OK"
	else
		error_cleanup "ldns-testpkts is different in ldns and unbound"
	fi
fi
cd $temp_dir/unbound

find . -name .c-mode-rc.el -exec rm {} \;
find . -name .cvsignore -exec rm {} \;
rm makedist.sh || error_cleanup "Failed to remove makedist.sh."

info "Determining Unbound version."
version=`./configure --version | head -1 | awk '{ print $3 }'` || \
    error_cleanup "Cannot determine version number."

info "Unbound version: $version"

if [ "$SNAPSHOT" = "yes" ]; then
    info "Building Unbound snapshot."
    version="$version-`date +%Y%m%d`"
    info "Snapshot version number: $version"
fi

replace_all doc/README
replace_all doc/unbound.8.in
replace_all doc/unbound.conf.5.in
replace_all doc/unbound-checkconf.8.in
replace_all doc/unbound-host.1
replace_all doc/libunbound.3.in

info "Renaming Unbound directory to unbound-$version."
cd ..
mv unbound unbound-$version || error_cleanup "Failed to rename unbound directory."

tarfile="../unbound-$version.tar.gz"

if [ -f $tarfile ]; then
    (question "The file $tarfile already exists.  Overwrite?" \
        && rm -f $tarfile) || error_cleanup "User abort."
fi

info "Creating tar unbound-$version.tar.gz"
tar czf ../unbound-$version.tar.gz unbound-$version || error_cleanup "Failed to create tar file."

cleanup

case $OSTYPE in
        linux*)
                sha=`sha1sum unbound-$version.tar.gz |  awk '{ print $1 }'`
                sha256=`sha256sum unbound-$version.tar.gz |  awk '{ print $1 }'`
                ;;
        freebsd*)
                sha=`sha1  unbound-$version.tar.gz |  awk '{ print $5 }'`
                sha256=`sha256  unbound-$version.tar.gz |  awk '{ print $5 }'`
                ;;
	*)
                sha=`sha1sum unbound-$version.tar.gz |  awk '{ print $1 }'`
                sha256=`sha256sum unbound-$version.tar.gz |  awk '{ print $1 }'`
                ;;
esac
echo $sha > unbound-$version.tar.gz.sha1
echo $sha256 > unbound-$version.tar.gz.sha256

info "Unbound distribution created successfully."
info "SHA1sum: $sha"

