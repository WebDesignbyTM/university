for i in {1..100} 
do
    sudo python3 tester.py docker valgrind | grep -i $1
done