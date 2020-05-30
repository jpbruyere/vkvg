#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "usage: batchBuild.sh [build directory]"
fi
cd $1
pwd
for file in test_*
do
    ./"$file" 1 100
done
