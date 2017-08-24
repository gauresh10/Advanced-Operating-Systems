#!/bin/bash

echo "Enter Scheduler Number "

echo "0 : O(1) Scheduler"

echo "1 : Credit Based Scheduler "

read num

if (("$num" == "1" ))||(( "$num" =="0")); then 
	echo $num
else
	echo "Please Enter Number 1 or 0"
	exit
fi


echo "Make Clean"
make clean

echo "Make matrix"
make
make matrix


for i in {1}
do
	echo "run the matrix"
	./bin/./matrix $num
done



killall matrix

echo "All done"
