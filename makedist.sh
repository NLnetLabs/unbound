#!/bin/sh

# Build unbound distribution tar from the git repository.
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
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Abort script on unexpected errors.
set -e

# Remember the current working directory.
cwd=`pwd`

# Utility functions.
usage () {
    cat >&2 <<EOF
Usage $0: [-h] [-s] [-u git_url] [-b git_branch] [-w ...args...]
Generate a distribution tar file for unbound.

    -h                  This usage information.
    -s                  Build a snapshot distribution file.  The current date is
                        automatically appended to the current unbound version number.
    -rc <nr>            Build a release candidate, the given string will be added
                        to the version number
                        (which will then be unbound-<version>rc<number>)
    -u git_url          Retrieve the source from the specified repository url.
                        Detected from the working copy if not specified.
    -b git_branch       Retrieve the specified branch or tag.
                        Detected from the working copy if not specified.
    -wssl openssl.xx.tar.gz Also build openssl from tarball for windows dist.
    -wxp expat.xx.tar.gz Also build expat from tarball for windows dist.
    -wdir directory     Build openssl and expat in a persistent directory for
                        windows dist. If builds are already in that directory
                        they are used right away. Useful when debuggin windows
                        builds.
    -w32                32bit windows compile.
    -w ...              Build windows binary dist. last args passed to configure.
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

replace_version () {
    local v1=`echo $2 | sed -e 's/^.*\..*\.//'`
    local v2=`echo $3 | sed -e 's/^.*\..*\.//'`
    replace_text "$1" "VERSION_MICRO\],\[$v1" "VERSION_MICRO\],\[$v2"
}
    
check_git_repo () {
    # Check if git repo and branch are specified.
    if [ -z "$GITREPO" ]; then
	if git status 2>&1 | grep "not a git repository" >/dev/null; then
		error "specify repo (using -u) or use settings detected by starting from working copy directory"
	else
	     GITREPO="`git config --get remote.origin.url`"
	fi
    fi
    if [ -z "$GITBRANCH" ]; then
	if git status 2>&1 | grep "not a git repository" >/dev/null; then
		error "specify branch (using -b) or use settings detected by starting from working copy directory"
	else
	     GITBRANCH="`git branch | grep '^\*' | sed -e 's/^\* //'`"
	fi
    fi
}

create_temp_dir () {
    # Creating temp directory
    info "Creating temporary working directory"
    temp_dir=`mktemp -d unbound-dist-XXXXXX`
    info "Directory '$temp_dir' created."
    cd $temp_dir
}

activate_windebug_dir () {
    info "Activating persistent directory for windows build."
    if test ! -d "$WINDIR"; then
        mkdir -p "$WINDIR"
        info "Created $WINDIR persistent directory."
    fi
    cd "$WINDIR"
    WINDIR="`pwd`"
}

# pass filename as $1 arg.
# creates file.sha1 and file.sha256
storehash () {
    case $OSTYPE in
        linux*)
                sha=`sha1sum $1 |  awk '{ print $1 }'`
                sha256=`sha256sum $1 |  awk '{ print $1 }'`
                ;;
        freebsd*)
                sha=`sha1 $1 |  awk '{ print $5 }'`
                sha256=`sha256 $1 |  awk '{ print $5 }'`
                ;;
	*)
		# in case $OSTYPE is gone.
		case `uname` in
		Linux*)
		  sha=`sha1sum $1 |  awk '{ print $1 }'`
		  sha256=`sha256sum $1 |  awk '{ print $1 }'`
		  ;;
		FreeBSD*)
		  sha=`sha1 $1 |  awk '{ print $5 }'`
		  sha256=`sha256 $1 |  awk '{ print $5 }'`
		  ;;
		*)
		  sha=`sha1sum $1 |  awk '{ print $1 }'`
		  sha256=`sha256sum $1 |  awk '{ print $1 }'`
		  ;;
		esac
                ;;
    esac
    echo $sha > $1.sha1
    echo $sha256 > $1.sha256
    echo "hash of $1.{sha1,sha256}"
    echo "sha1 $sha"
    echo "sha256 $sha256"
}


SNAPSHOT="no"
RC="no"
DOWIN="no"
W64="yes"
WINSSL=""
WINEXPAT=""
WINDIR=""
# Use environment if set otherwise null
MINJ="${MINJ:+$MINJ}"

# Parse the command line arguments.
while [ "$1" ]; do
    case "$1" in
        "-h")
            usage
            ;;
        "-u")
            GITREPO="$2"
            shift
            ;;
        "-b")
            GITBRANCH="$2"
            shift
            ;;
        "-s")
            SNAPSHOT="yes"
            ;;
        "-wssl")
	    WINSSL="$2"
	    shift
	    ;;
        "-wxp")
	    WINEXPAT="$2"
	    shift
	    ;;
        "-wdir")
	    WINDIR="$2"
	    shift
	    ;;
	"-w32")
	    W64="no"
	    ;;
        "-w")
            DOWIN="yes"
	    shift
	    break
            ;;
        "-rc")
            RC="$2"
            shift
            ;;
        *)
            error "Unrecognized argument -- $1"
            ;;
    esac
    shift
done

if [ "$DOWIN" = "yes" ]; then
    # detect crosscompile, from Fedora13 at this point.
    if test "`uname`" = "Linux"; then 
	info "Crosscompile windows dist"
        cross="yes"
	if test "$W64" = "yes"; then
		warch="x86_64"   # i686 for 32bit, or x86_64 for 64bit
		mw64="mingw64"   # mingw32 or mingw64
	else
		warch="i686"
		mw64="mingw32"
	fi
	configure="${mw64}-configure"   # mingw32-configure, mingw64-configure
	strip="${warch}-w64-mingw32-strip"
	makensis="makensis"	# from mingw32-nsis package
	# flags for crosscompiled dependency libraries
	cross_flag=""
	shared_cross_flag=""

	check_git_repo

	if test -n "$WINDIR"; then
		activate_windebug_dir
	else
		create_temp_dir
	fi

	if test -n "$WINSSL" \
		-a -n "$WINDIR" \
		-a -d "$WINDIR" \
		-a -d "${WINDIR}/sslinstall" \
		-a -d "${WINDIR}/openssl_shared" \
		-a -d "${WINDIR}/sslsharedinstall"; then
		info "Found already compiled openssl at $WINDIR/sslinstall; using that."
		WINSSL=""
		# Variables needed later on.
		sslinstall="${WINDIR}/sslinstall"
		cross_flag="$cross_flag --with-ssl=$sslinstall"
		sslsharedinstall="${WINDIR}/sslsharedinstall"
		shared_cross_flag="$shared_cross_flag --with-ssl=$sslsharedinstall"
	fi
	# crosscompile openssl for windows.
	if test -n "$WINSSL"; then
		info "Cross compile $WINSSL"
		info "winssl tar unpack"
		(cd ..; gzip -cd $WINSSL) | tar xf - || error_cleanup "tar unpack of $WINSSL failed"
		sslinstall="`pwd`/sslinstall"
		cp -r openssl-* openssl_shared
		cd openssl-* || error_cleanup "no openssl-X dir in tarball"
		# configure for crosscompile, without CAPI because it fails
		# cross-compilation and it is not used anyway
		# before 1.0.1i need --cross-compile-prefix=i686-w64-mingw32-
		if test "$mw64" = "mingw64"; then
			sslflags="no-asm no-tests -DOPENSSL_NO_CAPIENG mingw64"
			sspdll="/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libssp-0.dll"
		else
			sslflags="no-asm no-tests -DOPENSSL_NO_CAPIENG mingw"
			sspdll="/usr/i686-w64-mingw32/sys-root/mingw/bin/libssp-0.dll"
		fi
		if test -f "$sspdll"; then
			# stack protector lib needs to link in to make
			# -lws2_32 work in openssl link stage
			SSPLIB="-l:libssp.a"
		else
			# disable SSPLIB if no such file
			SSPLIB=""
		fi
		info "winssl: Configure no-shared $sslflags"
		if test "$W64" = "no"; then
			# Disable stack-protector for 32-bit windows builds.
			# mingw passes an LDFLAGS, so there is something
			# passed in the LDFLAGS to stop -lssp passed in it.
			set -x # echo the configure command
			__CNF_LDLIBS=$SSPLIB __CNF_LDFLAGS="-fno-stack-protector" CC=${warch}-w64-mingw32-gcc AR=${warch}-w64-mingw32-ar RANLIB=${warch}-w64-mingw32-ranlib WINDRES=${warch}-w64-mingw32-windres ./Configure --prefix="$sslinstall" no-shared $sslflags || error_cleanup "OpenSSL Configure failed"
			set +x
		else
			set -x # echo the configure command
			__CNF_LDLIBS=$SSPLIB CC=${warch}-w64-mingw32-gcc AR=${warch}-w64-mingw32-ar RANLIB=${warch}-w64-mingw32-ranlib WINDRES=${warch}-w64-mingw32-windres ./Configure --prefix="$sslinstall" no-shared $sslflags || error_cleanup "OpenSSL Configure failed"
			set +x
		fi
		info "winssl: make"
		make $MINJ || error_cleanup "OpenSSL crosscompile failed"
		# only install sw not docs, which take a long time.
		info "winssl: make install_sw"
		make install_sw || error_cleanup "OpenSSL install failed"
		cross_flag="$cross_flag --with-ssl=$sslinstall"
		cd ..

		# shared compile
		sslsharedinstall="`pwd`/sslsharedinstall"
		cd openssl_shared
		info "winssl: Configure shared $sslflags"
		if test "$W64" = "no"; then
			set -x # echo the configure command
			__CNF_LDLIBS=$SSPLIB __CNF_LDFLAGS="-fno-stack-protector" CC=${warch}-w64-mingw32-gcc AR=${warch}-w64-mingw32-ar RANLIB=${warch}-w64-mingw32-ranlib WINDRES=${warch}-w64-mingw32-windres ./Configure --prefix="$sslsharedinstall" shared $sslflags || error_cleanup "OpenSSL Configure failed"
			set +x
		else
			set -x # echo the configure command
			__CNF_LDLIBS=$SSPLIB CC=${warch}-w64-mingw32-gcc AR=${warch}-w64-mingw32-ar RANLIB=${warch}-w64-mingw32-ranlib WINDRES=${warch}-w64-mingw32-windres ./Configure --prefix="$sslsharedinstall" shared $sslflags || error_cleanup "OpenSSL Configure failed"
			set +x
		fi
		info "winssl: make"
		make $MINJ || error_cleanup "OpenSSL crosscompile failed"
		info "winssl: make install_sw"
		make install_sw || error_cleanup "OpenSSL install failed"
		shared_cross_flag="$shared_cross_flag --with-ssl=$sslsharedinstall"
		cd ..
	fi

	if test -n "$WINEXPAT" \
		-a -n "$WINDIR" \
		-a -d "$WINDIR" \
		-a -d "${WINDIR}/wxpinstall"; then
		info "Found already compiled expat at $WINDIR/wxpinstall; using that."
		WINEXPAT=""
		# Variables needed later on.
		wxpinstall="${WINDIR}/wxpinstall"
		cross_flag="$cross_flag --with-libexpat=$wxpinstall"
		shared_cross_flag="$shared_cross_flag --with-libexpat=$wxpinstall"
	fi
	if test -n "$WINEXPAT"; then
		info "Cross compile $WINEXPAT"
		info "wxp: tar unpack"
		(cd ..; bzip2 -cd $WINEXPAT) | tar xf - || error_cleanup "tar unpack of $WINEXPAT failed"
		wxpinstall="`pwd`/wxpinstall"
		cd expat-* || error_cleanup "no expat-X dir in tarball"
		info "wxp: configure"
		if test "$W64" = "no"; then
			# Disable stack-protector for 32-bit windows builds.
			set -x # echo the configure command
			$configure --prefix="$wxpinstall" --exec-prefix="$wxpinstall" --bindir="$wxpinstall/bin" --includedir="$wxpinstall/include" --mandir="$wxpinstall/man" --libdir="$wxpinstall/lib" LDFLAGS="-fno-stack-protector" || error_cleanup "libexpat configure failed"
			set +x
		else
			set -x # echo the configure command
			$configure --prefix="$wxpinstall" --exec-prefix="$wxpinstall" --bindir="$wxpinstall/bin" --includedir="$wxpinstall/include" --mandir="$wxpinstall/man" --libdir="$wxpinstall/lib"  || error_cleanup "libexpat configure failed"
			set +x
		fi
		info "wxp: make"
		make $MINJ || error_cleanup "libexpat crosscompile failed"
		info "wxp: make install"
		make install || error_cleanup "libexpat install failed"
		cross_flag="$cross_flag --with-libexpat=$wxpinstall"
		shared_cross_flag="$shared_cross_flag --with-libexpat=$wxpinstall"
		cd ..
	fi

	if test -n "$WINDIR"; then
		cd "$cwd"
		create_temp_dir
	fi
	info "GITREPO   is $GITREPO"
	info "GITBRANCH is $GITBRANCH"
	info "Exporting source from git."
	info "git clone --depth=1 --no-tags -b $GITBRANCH $GITREPO unbound"
	git clone --depth=1 --no-tags -b $GITBRANCH $GITREPO unbound || error_cleanup "git clone failed"
	cd unbound || error_cleanup "Unbound not exported correctly from git"
	rm -rf .git .travis.yml .gitattributes .github .gitignore || error_cleanup "Failed to remove .git tracking and ci information"

	# on a re-configure the cache may no longer be valid...
	if test -f mingw32-config.cache; then rm mingw32-config.cache; fi
    else 
	cross="no"	# mingw and msys
	cross_flag=""
	configure="./configure"
	strip="strip"
	makensis="c:/Program Files/NSIS/makensis.exe" # http://nsis.sf.net
    fi

    # version gets compiled into source, edit the configure to set  it
    version=`./configure --version | head -1 | awk '{ print $3 }'` \
	|| error_cleanup "Cannot determine version number."
    if [ "$RC" != "no" -o "$SNAPSHOT" != "no" ]; then
    	if [ "$RC" != "no" ]; then
    		version2=`echo $version | sed -e 's/rc.*$//' -e 's/_20.*$//'`
		version2=`echo $version2 | sed -e 's/rc.*//'`"rc$RC"
	fi
    	if [ "$SNAPSHOT" != "no" ]; then
    		version2=`echo $version | sed -e 's/rc.*$//' -e 's/_20.*$//'`
    		version2="${version2}_`date +%Y%m%d`"
	fi
	replace_version "configure.ac" "$version" "$version2"
    	version="$version2"
    	info "Rebuilding configure script (autoconf) snapshot."
	autoconf -f || error_cleanup "Autoconf failed."
	autoheader -f || error_cleanup "Autoheader failed."
    	rm -r autom4te* || echo "ignored"
	rm -f config.h.in~ || echo "ignore absence of config.h.in~ file."
    fi

    if test "`uname`" = "Linux"; then 
	    cd ..
	    cp -r unbound unbound_shared
	    unbound_shared="`pwd`/unbound_shared"
	    cd unbound
    fi

    # procedure for making unbound installer on mingw. 
    info "Creating windows dist unbound $version"
    info "Calling configure"
    if test "$W64" = "no"; then
	file_flag="--with-conf-file=C:\Program Files (x86)\Unbound\service.conf"
	file2_flag="--with-rootkey-file=C:\Program Files (x86)\Unbound\root.key"
	file3_flag="--with-rootcert-file=C:\Program Files (x86)\Unbound\icannbundle.pem"
	version="$version"-w32
    fi
    if test "$W64" = "no"; then
		# Disable stack-protector for 32-bit windows builds.
		set -x
		$configure --enable-debug --enable-static-exe --disable-flto --disable-gost $* $cross_flag "$file_flag" "$file2_flag" "$file3_flag" CFLAGS='-O2 -g -fno-stack-protector' LDFLAGS="-fno-stack-protector" \
		|| error_cleanup "Could not configure"
		set +x
    else
		set -x
		$configure --enable-debug --enable-static-exe --disable-flto --disable-gost $* $cross_flag \
		|| error_cleanup "Could not configure"
		set +x
    fi
    info "Calling make"
	make $MINJ || error_cleanup "Could not make"
    info "Make complete"

    if test "`uname`" = "Linux"; then 
    info "Make DLL"
    cd $unbound_shared
    if test "$W64" = "no"; then
	# Disable stack-protector for 32-bit windows builds.
		set -x
		$configure --enable-debug --disable-flto --disable-gost $* $shared_cross_flag "$file_flag" "$file2_flag" "$file3_flag" CFLAGS='-O2 -g -fno-stack-protector' LDFLAGS="-fno-stack-protector" \
		|| error_cleanup "Could not configure"
		set +x
    else
		set -x
		$configure --enable-debug --disable-flto --disable-gost $* $shared_cross_flag \
		|| error_cleanup "Could not configure"
		set +x
    fi
    info "Calling make for DLL"
	make $MINJ || error_cleanup "Could not make DLL"
    info "Make DLL complete"
    cd ../unbound
    fi

    info "Unbound version: $version"
    file="unbound-$version.zip"
    rm -f $file
    info "Creating $file"
    grep '^". IN DS' smallapp/unbound-anchor.c | sed -e 's/"//' -e 's/\\n.*$//' > root.key
    mkdir tmp.$$
    # keep debug symbols
    #$strip unbound.exe
    #$strip anchor-update.exe
    #$strip unbound-control.exe
    #$strip unbound-host.exe
    #$strip unbound-anchor.exe
    #$strip unbound-checkconf.exe
    #$strip unbound-service-install.exe
    #$strip unbound-service-remove.exe
    cd tmp.$$
    cp ../root.key .
    cp ../doc/example.conf ../doc/Changelog .
    cp ../unbound.exe ../unbound-anchor.exe ../unbound-host.exe ../unbound-control.exe ../unbound-checkconf.exe ../unbound-service-install.exe ../unbound-service-remove.exe ../LICENSE ../winrc/unbound-control-setup.cmd ../winrc/unbound-website.url ../winrc/service.conf ../winrc/README.txt ../contrib/create_unbound_ad_servers.cmd ../contrib/warmup.cmd ../contrib/unbound_cache.cmd .
    mkdir libunbound
    # test to see if lib or lib64 (for openssl 3.0.0) needs to be used
    if test -f $sslsharedinstall/lib/libcrypto.dll.a; then
	cp $sslsharedinstall/lib/libcrypto.dll.a libunbound/.
    else
	cp $sslsharedinstall/lib64/libcrypto.dll.a libunbound/.
    fi
    if test -f $sslsharedinstall/lib/libssl.dll.a; then
	cp $sslsharedinstall/lib/libssl.dll.a libunbound/.
    else
	cp $sslsharedinstall/lib64/libssl.dll.a libunbound/.
    fi
    cp $unbound_shared/unbound.h               \
	$unbound_shared/.libs/libunbound*.dll  \
	$unbound_shared/.libs/libunbound.dll.a \
	$unbound_shared/.libs/libunbound.a     \
	$unbound_shared/.libs/libunbound*.def  \
	$sslsharedinstall/bin/libcrypto*.dll   \
	$sslsharedinstall/bin/libssl*.dll      \
	$wxpinstall/bin/libexpat*.dll          \
	$wxpinstall/lib/libexpat.dll.a         \
	libunbound/.
    if test -f "$sspdll"; then
	    cp "$sspdll" libunbound/.
    fi
    # zipfile
    zip -r ../$file LICENSE README.txt unbound.exe unbound-anchor.exe unbound-host.exe unbound-control.exe unbound-checkconf.exe unbound-service-install.exe unbound-service-remove.exe unbound-control-setup.cmd example.conf service.conf root.key unbound-website.url create_unbound_ad_servers.cmd warmup.cmd unbound_cache.cmd Changelog libunbound
    info "Testing $file"
    (cd .. ; zip -T $file )
    # installer
    info "Creating installer"
    quadversion=`cat ../config.h | grep RSRC_PACKAGE_VERSION | sed -e 's/#define RSRC_PACKAGE_VERSION //' -e 's/,/\\./g'`
    cat ../winrc/setup.nsi | sed -e 's/define VERSION.*$/define VERSION "'$version'"/' -e 's/define QUADVERSION.*$/define QUADVERSION "'$quadversion'"/' > ../winrc/setup_ed.nsi
    if test "$W64" = "no"; then
	mv ../winrc/setup_ed.nsi ../winrc/setup_ed_old.nsi
	cat ../winrc/setup_ed_old.nsi | sed -e 's/PROGRAMFILES64/PROGRAMFILES/' -e 's/SetRegView 64/SetRegView 32/' > ../winrc/setup_ed.nsi
    fi
    "$makensis" ../winrc/setup_ed.nsi
    info "Created installer"
    cd ..
    rm -rf tmp.$$
    mv winrc/unbound_setup_$version.exe .
    if test "$cross" = "yes"; then
	    mv unbound_setup_$version.exe $cwd/.
	    mv unbound-$version.zip $cwd/.
	    cleanup
    fi
    storehash unbound_setup_$version.exe
    storehash unbound-$version.zip
    ls -lG unbound_setup_$version.exe
    ls -lG unbound-$version.zip

    echo "create signed versions with:"
    echo "  gpg --armor --detach-sign --digest-algo SHA256 unbound_setup_$version.exe"
    echo "  gpg --armor --detach-sign --digest-algo SHA256 unbound-$version.zip"

    info "Done"
    exit 0
fi

check_git_repo

# Start the packaging process.
info "GITREPO   is $GITREPO"
info "GITBRANCH is $GITBRANCH"
info "SNAPSHOT  is $SNAPSHOT"

#question "Do you wish to continue with these settings?" || error "User abort."

create_temp_dir

info "Exporting source from git."
# --depth=1 and --no-tags reduce the download size.
info "git clone --depth=1 --no-tags -b $GITBRANCH $GITREPO unbound"
git clone --depth=1 --no-tags -b $GITBRANCH $GITREPO unbound || error_cleanup "git clone failed"

cd unbound || error_cleanup "Unbound not exported correctly from git"
rm -rf .git .travis.yml .gitattributes .github .gitignore || error_cleanup "Failed to remove .git tracking and ci information"

info "Adding libtool utils (libtoolize)."
libtoolize -c --install || libtoolize -c || error_cleanup "Libtoolize failed."

# https://www.gnu.org/software/gettext/manual/html_node/config_002eguess.html
info "Updating config.guess and config.sub"
wget -O config.guess 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
wget -O config.sub 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
chmod a+x config.guess config.sub

# Remove quarantine bit on Apple platforms
if [ `uname -s | grep -i -c darwin` -ne 0 ]; then
    if [ -n `command -v xattr` ]; then
        xattr -d com.apple.quarantine config.guess
        xattr -d com.apple.quarantine config.sub
    fi
fi

info "Building configure script (autoreconf)."
autoreconf -f || error_cleanup "Autoconf failed."

rm -r autom4te* || error_cleanup "Failed to remove autoconf cache directory."
rm -f config.h.in~ || echo "ignore absence of config.h.in~ file."

info "Building lexer and parser."
echo "#include \"config.h\"" > util/configlexer.c || error_cleanup "Failed to create configlexer"
echo "#include \"util/configyyrename.h\"" >> util/configlexer.c || error_cleanup "Failed to create configlexer"
flex -i -t util/configlexer.lex >> util/configlexer.c  || error_cleanup "Failed to create configlexer"
if test -x `which bison` 2>&1; then YACC=bison; else YACC=yacc; fi
$YACC -y -d -o util/configparser.c util/configparser.y || error_cleanup "Failed to create configparser"

find . -name .c-mode-rc.el -exec rm {} \;
find . -name .cvsignore -exec rm {} \;
rm makedist.sh || error_cleanup "Failed to remove makedist.sh."

info "Determining Unbound version."
version=`./configure --version | head -1 | awk '{ print $3 }'` || \
    error_cleanup "Cannot determine version number."

info "Unbound version: $version"

RECONFIGURE="no"

if [ "$RC" != "no" ]; then
    info "Building Unbound release candidate $RC."
    version2="${version}rc$RC"
    info "Version number: $version2"

    replace_version "configure.ac" "$version" "$version2"
    version="$version2"
    RECONFIGURE="yes"
fi

if [ "$SNAPSHOT" = "yes" ]; then
    info "Building Unbound snapshot."
    version2="${version}_`date +%Y%m%d`"
    info "Snapshot version number: $version2"

    replace_version "configure.ac" "$version" "$version2"
    version="$version2"
    RECONFIGURE="yes"
fi

if [ "$RECONFIGURE" = "yes" ]; then
    info "Rebuilding configure script (autoconf) snapshot."
    autoreconf -f || error_cleanup "Autoconf failed."
    rm -r autom4te* || error_cleanup "Failed to remove autoconf cache directory."
    rm -f config.h.in~ || echo "ignore absence of config.h.in~ file."
fi

replace_all doc/README
replace_all doc/unbound.8.in
replace_all doc/unbound.conf.5.in
replace_all doc/unbound-checkconf.8.in
replace_all doc/unbound-control.8.in
replace_all doc/unbound-anchor.8.in
replace_all doc/unbound-host.1.in
replace_all doc/example.conf.in
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

storehash unbound-$version.tar.gz
echo "create unbound-$version.tar.gz.asc with:"
echo "  gpg --armor --detach-sign --digest-algo SHA256 unbound-$version.tar.gz"
echo "  gpg --armor --detach-sign --digest-algo SHA256 unbound-$version.zip"
echo "  gpg --armor --detach-sign --digest-algo SHA256 unbound_setup_$version.exe"

info "Unbound distribution created successfully."
