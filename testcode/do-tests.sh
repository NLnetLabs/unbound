#!/usr/bin/env bash

NEED_SPLINT='00-lint.tpkg'
NEED_DOXYGEN='01-doc.tpkg'
NEED_LDNS_TESTNS='fwd_no_edns.tpkg fwd_tcp_tc.tpkg fwd_tcp.tpkg fwd_three_service.tpkg fwd_three.tpkg fwd_ttlexpire.tpkg fwd_udp.tpkg fwd_tcp_tc6.tpkg fwd_compress_c00c.tpkg fwd_ancil.tpkg stat_timer.tpkg 05-asynclook.tpkg stream_tcp.tpkg speed_cache.tpkg fwd_oneport.tpkg fwd_udptmout.tpkg fwd_waitudp.tpkg tcp_sigpipe.tpkg hostsfileosx.tpkg local_nodefault.tpkg fwd_zero.tpkg'
NEED_XXD='fwd_compress_c00c.tpkg fwd_zero.tpkg'
NEED_NC='fwd_compress_c00c.tpkg fwd_zero.tpkg'
NEED_CURL='06-ianaports.tpkg'
NEED_WHOAMI='07-confroot.tpkg'
NEED_IPV6='fwd_ancil.tpkg fwd_tcp_tc6.tpkg stub_udp6.tpkg'
NEED_NOMINGW='tcp_sigpipe.tpkg 07-confroot.tpkg 08-host-lib.tpkg fwd_ancil.tpkg'

# test if dig, wdiff and ldns-testns are available.
if test ! -x "`which dig 2>&1`"; then echo No 'dig' in path; exit 1; fi
if test ! -x "`which wdiff 2>&1`"; then echo No 'wdiff' in path; exit 1; fi
if test ! -x "`which ldns-testns 2>&1`"; then echo No 'ldns-testns' in path; exit 1; fi

# test for ipv6, uses streamptcp peculiarity.
if ./streamtcp -f ::1 2>&1 | grep "not supported" >/dev/null 2>&1; then
	HAVE_IPV6=no
else
	HAVE_IPV6=yes
fi

# test mingw. no signals and so on.
if uname | grep MINGW >/dev/null; then
	HAVE_MINGW=yes
else
	HAVE_MINGW=no
fi

cd testdata;
sh ../testcode/mini_tpkg.sh clean
rm -f .perfstats.txt
for test in `ls *.tpkg`; do
	SKIP=0
	if echo $NEED_SPLINT | grep $test >/dev/null; then
		if test ! -x "`which splint`"; then
			SKIP=1;
		fi
	fi
	if echo $NEED_DOXYGEN | grep $test >/dev/null; then
		if test ! -x "`which doxygen`"; then
			SKIP=1;
		fi
	fi
	if echo $NEED_CURL | grep $test >/dev/null; then
		if test ! -x "`which curl`"; then
			SKIP=1;
		fi
	fi
	if echo $NEED_LDNS_TESTNS | grep $test >/dev/null; then
		if test ! -x "`which ldns-testns`"; then
			SKIP=1;
		fi
	fi
	if echo $NEED_XXD | grep $test >/dev/null; then
		if test ! -x "`which xxd`"; then
			SKIP=1;
		fi
	fi
	if echo $NEED_NC | grep $test >/dev/null; then
		if test ! -x "`which nc`"; then
			SKIP=1;
		fi
	fi
	if echo $NEED_WHOAMI | grep $test >/dev/null; then
		if test ! -x "`which whoami`"; then
			SKIP=1;
		fi
	fi
	if echo $NEED_IPV6 | grep $test >/dev/null; then
		if test "$HAVE_IPV6" = no; then
			SKIP=1;
		fi
	fi
	if echo $NEED_NOMINGW | grep $test >/dev/null; then
		if test "$HAVE_MINGW" = yes; then
			SKIP=1;
		fi
	fi
	if test $SKIP -eq 0; then
		echo $test
		sh ../testcode/mini_tpkg.sh -a ../.. exe $test
	else
		echo "skip $test"
	fi
done
sh ../testcode/mini_tpkg.sh report
cat .perfstats.txt
