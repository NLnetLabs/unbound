#!/usr/bin/env bash

NEED_SPLINT='00-lint.tpkg'
NEED_DOXYGEN='01-doc.tpkg'
NEED_LDNS_TESTNS='fwd_no_edns.tpkg fwd_tcp_tc.tpkg fwd_tcp.tpkg fwd_three_service.tpkg fwd_three.tpkg fwd_ttlexpire.tpkg fwd_udp.tpkg fwd_tcp_tc6.tpkg fwd_compress_c00c.tpkg fwd_ancil.tpkg stat_timer.tpkg 05-asynclook.tpkg stream_tcp.tpkg'
NEED_XXD='fwd_compress_c00c.tpkg'
NEED_NC='fwd_compress_c00c.tpkg'

cd testdata;
sh ../testcode/mini_tpkg.sh clean
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
	if test $SKIP -eq 0; then
		echo $test
		sh ../testcode/mini_tpkg.sh -a ../.. exe $test
	else
		echo "skip $test"
	fi
done
sh ../testcode/mini_tpkg.sh report
