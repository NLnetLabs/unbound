# tpkg that only exes the files.
args="../.."
if test "$1" = "-a"; then
	args=$2
	shift
	shift
fi
	
if test "$1" = "clean"; then
	echo "rm -f result.* .done* .tpkg.var.master .tpkg.var.test"
	rm -f result.* .done* .tpkg.var.master .tpkg.var.test
	exit 0
fi
if test "$1" = "fake"; then
	echo "minitpkg fake $2"
	echo "fake" > .done-`basename $2 .tpkg`
	exit 0
fi
if test "$1" = "report" || test "$2" = "report"; then
	echo "Minitpkg Report"
	for result in result.*; do
		name=`echo $result | sed -e 's/result\.//'`
		if test -f ".done-$name"; then
			if test "$1" != "-q"; then
				echo "** PASSED ** : $name"
			fi
		else
			echo "!! FAILED !! : $name"
		fi
	done
	exit 0
fi

if test "$1" != 'exe'; then
	# usage
	echo "mini tpkg. Reduced functionality for old shells."
	echo "	tpkg exe <file>"
	echo "	tpkg fake <file>"
	echo "	tpkg clean"
	echo "	tpkg [-q] report"
	exit 1
fi
shift

# do not execute if the disk is too full
DISKLIMIT=100000
avail=`df . | tail -1 | awk '{print $4}'`
if test "$avail" -lt "$DISKLIMIT"; then
	echo "minitpkg: The disk is too full! Only $avail."
	exit 1
fi

name=`basename $1 .tpkg`
dir=$name.$$
result=result.$name
done=.done-$name
success="no"
shell="bash"

# check already done
if test -f .done-$name; then
	echo "minitpkg .done-$name exists. skip test."
	exit 0
fi

# Extract
echo "minitpkg extract $1 to $dir"
mkdir $dir
gzip -cd $name.tpkg | (cd $dir; tar xf -)
cd $dir
mv $name.dir/* .

# EXE
echo "minitpkg exe $name"
echo "minitpkg exe $name" > $result
if test -f $name.pre; then
	echo "minitpkg exe $name.pre"
	echo "minitpkg exe $name.pre" >> $result
	$shell $name.pre $args >> $result
	if test $? -ne 0; then
		echo "Warning: $name.pre did not exit successfully"
	fi
fi
if test -f $name.test; then
	echo "minitpkg exe $name.test"
	echo "minitpkg exe $name.test" >> $result
	$shell $name.test $args >>$result 2>&1
	if test $? -ne 0; then
		echo "$name: FAILED" >> $result
		echo "$name: FAILED"
		success="no"
	else
		echo "$name: PASSED" >> $result
		echo "$name: PASSED" > ../.done-$name
		echo "$name: PASSED"
		success="yes"
	fi
fi
if test -f $name.post; then
	echo "minitpkg exe $name.post"
	echo "minitpkg exe $name.post" >> $result
	$shell $name.post $args >> $result
	if test $? -ne 0; then
		echo "Warning: $name.post did not exit successfully"
	fi
fi

mv $result ..
cd ..
rm -rf $dir
