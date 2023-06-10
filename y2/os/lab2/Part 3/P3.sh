#!/bin/bash

ALLFILES=0
FFILES=0
DIR=0

# parcurge toate path-urile si numara in ALLFILES 
# cate fisiere gaseste
list_files() {
	
	# normal files
	for file in "$1"/*
	do
		if [ "${file: - 1}" = \* ]
		then
			continue
		fi

		if [ -d "$file" ]
		then
			list_files "$file"
		else
			echo $file
			ALLFILES=`expr $ALLFILES + 1`
		fi
	done

	# hidden files
	for file in "$1"/.*
	do
		if [ "${file: - 1}" = . ]
		then
			continue
		fi

		if [ "${file: - 1}" = \* ]
		then
			continue
		fi

		if [ -d "$file" ]
		then
			list_files "$file"
		else
			echo "$file"
			ALLFILES=`expr $ALLFILES + 1`
		fi
	done
}

# foloseste un fisier parametru ca contine output-ul
# functiei list_files pentru a parsa fisierele ce
# indeplinesc un format si le numara in FFILES
list_pattern_files() {
	FFILES=`cat tmp.txt | grep -E "$1" | wc -l`
	echo There are $FFILES files that meet the criteria
	cat tmp.txt | grep -E "$1"
}

list_subdirs() {
	file="tmp.txt"
	subdirs=()
	while read -r line
	do
		subdirs+=("${line%/*}")
		# echo $line
	done < $file

	DIR=`printf "%s\n" "${subdirs[@]}" | sort -u | wc -l`
	echo $DIR subdirectories have been found
	printf "%s\n" "${subdirs[@]}" | sort -u
}

if [ $# -ne 1 ]
then
	echo "Usage: $0 <dir-name>"
	exit
fi

echo The given path is $1
echo

if ! [ -d "$1" ]
then
	echo "Parameter is not a directory"
	exit
fi

touch tmp.txt
list_files $1 > tmp.txt

echo $ALLFILES total files have been found
cat tmp.txt	
echo

list_pattern_files /f.+c$
echo

list_subdirs
echo

mkdir $DIR
touch $DIR/$ALLFILES
echo $FFILES > $DIR/$ALLFILES

rm tmp.txt