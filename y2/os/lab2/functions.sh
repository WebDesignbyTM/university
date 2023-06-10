add_num() {
	if [[ $# -lt 2 ]]
	then
		echo "Please input two numbers"
		return
	fi

	echo `eval $1 + $2` 
}
