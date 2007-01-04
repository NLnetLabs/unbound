#!/usr/bin/env bash

cd testdata;
for test in `ls *.tpkg`; do
	echo $test
	tpkg -a ../.. exe $test
done
