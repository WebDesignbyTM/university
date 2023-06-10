#!/bin/bash
# Write a shell script named "ls_param.sh" that displays on the screen every N-th parameter from its command line, in the order they were specified. N is given as its first command line parameter. You may use eval command. You can begin with the case N=1. Deal with the following cases:
if test $# -lt 1
then 
	echo "Please input arguments"
	exit
fi

for ((i = $1 ; i <= $# ; i += $1))
do
  echo ${!i}
done
