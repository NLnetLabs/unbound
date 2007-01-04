#!/usr/bin/env bash

NEED_SPLINT='00-lint.tpkg'
NEED_DOXYGEN='01-doc.tpkg'

cd testdata;
for test in `ls *.tpkg`; do
	SKIP=0
	if echo $NEED_SPLINT | grep $test >/dev/null; then
		if which splint >/dev/null 2>&1; then
			:
		else
			SKIP=1;
		fi
	fi
	if echo $NEED_DOXYGEN | grep $test >/dev/null; then
		if which doxygen >/dev/null 2>&1; then
			:
		else
			SKIP=1;
		fi
	fi
	if test $SKIP -eq 0; then
		echo $test
		tpkg -a ../.. exe $test
	else
		echo "skip $test"
	fi
done
