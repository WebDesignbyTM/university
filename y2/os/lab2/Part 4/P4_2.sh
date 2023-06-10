#!/bin/bash

# nu am apucat sa termin partea a doua; e ceva problema cu stat
file_info() {

	if [ $# -ne 1 ]
	then
		echo Please input a file_info
		return
	fi

	size=`stat -c %s $1`
	size=`numfmt --from=none --to=iec $size`
	printf "owner uid\nsize\n"
	printf "%s %s\n" `stat -c "%U %u" $1` $size
}

limit_search() {

	echo Currently in $1

	for fname in "$1"/*
	do
		echo $fname
		echo

		username=`stat -c %U \"$fname\"`
		if test "$username" = "$2" -a `stat -c %s \"$fname\"` -gt $3
		then
			echo Found $fname
		elif [ -d $fname ]
		then
			limit_search $fname $2 $3
		fi
	done

}

if [ $# -ne 3 ]
then
	echo "Usage $0 <dir-name> <username> <size>"
fi

limit_search $1 $2 $3