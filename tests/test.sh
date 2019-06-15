#!/bin/bash

set -e

cd ..
./build

cd tests
./test.py

echo '5001.txt ----------------------'
cat ./procedures/host1.txt
echo 'Log of 5001--------------------'
cat 5001.txt
echo

echo '5002.txt-----------------------'
cat ./procedures/host2.txt
echo 'Log of 5002--------------------'
cat 5002.txt
echo

echo '5003.txt-----------------------'
cat ./procedures/host3.txt
echo 'Log of 5003--------------------'
cat 5003.txt
echo

echo '5004.txt-----------------------'
cat ./procedures/host4.txt
echo 'Log of 5004--------------------'
cat 5004.txt

rm 500*.txt
