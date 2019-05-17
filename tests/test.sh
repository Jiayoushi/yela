#!/bin/bash

set -e

cd ..
./build

cd tests
./test.py

echo '5001.txt ----------------------'
cat ./procedures/host1.txt
cat 5001.txt
echo '5002.txt-----------------------'
cat ./procedures/host2.txt
cat 5002.txt
echo '5003.txt-----------------------'
cat ./procedures/host3.txt
cat 5003.txt
echo '5004.txt-----------------------'
cat ./procedures/host4.txt
cat 5004.txt

rm 500*.txt
