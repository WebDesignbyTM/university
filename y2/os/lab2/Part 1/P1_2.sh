echo "-----? Display the line number of each line containing the string \"character.\" at the end of line. "
echo "-----> grep -n -E \"character[.]$\" tests.txt | cut -d ':' -f 1"
grep -n -E "character[.]$" tests.txt | cut -d ':' -f 1
echo "-----? Also, display the number of lines corresponding to that criteria, using the wc command."
echo "grep -n -E \"character[.]$\" tests.txt | cut -d ':' -f 1 | wc -l"
grep -n -E "character[.]$" tests.txt | cut -d ':' -f 1 | wc -l
