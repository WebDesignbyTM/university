#!/bin/bash

recursive_search() {

	echo Currently in $1

	for fname in "$1"/*
	do
		if [[ "$fname" = *$2* ]]
		then
			echo Found $fname
		elif [ -d "$fname" ]
		then
			recursive_search "$fname" "$2"
		fi
	done

	for fname in "$1"/.*
	do
		if test "`basename \"$fname\"`" = "." -o "`basename \"$fname\"`" = ".."
		then
			continue
		fi

		if [[ "$fname" = *$2* ]]
		then
			echo Found $fname
		elif [ -d "$fname" ]
		then
			recursive_search "$fname" "$2"
		fi
	done
}

if [ $# -ne 2 ]
then
	echo "Usage: $0 <dir-name> <pattern>"
fi

recursive_search "$1" "$2"