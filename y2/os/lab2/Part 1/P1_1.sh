echo "-----? Display the line number of each line containing the string \"number\""
echo "-----$ grep -n number tests.txt | cut -d ':' -f 1"
grep -n number tests.txt | cut -d ':' -f 1
echo "-----> Also, display the number of lines corresponding to that criteria, using the wc command."
echo "-----$ grep -n number tests.txt | cut -d ':' -f 1 | wc -l"
grep -n number tests.txt | cut -d ':' -f 1 | wc -l

