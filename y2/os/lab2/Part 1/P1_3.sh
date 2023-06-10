echo "-----? Display the line number of each line containing numbers of two digits."
echo "-----> grep -n -E \"[0-9]{2}\" tests.txt | cut -d ':' -f 1"
grep -n -E "[0-9]{2}" tests.txt | cut -d ':' -f 1
echo "-----? Also, display the number of lines corresponding to that criteria, using the wc command."
echo "-----> grep -n -E \"[0-9]{2}\" tests.txt | cut -d ':' -f 1 | wc -l"
grep -n -E "[0-9]{2}" tests.txt | cut -d ':' -f 1 | wc -l

