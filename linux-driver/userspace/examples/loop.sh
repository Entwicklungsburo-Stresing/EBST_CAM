#!/bin/bash

# This script opens 4 terminal windows.

i="0"

while [ $i -lt 10000 ]
do
echo $i
./readout-blocking-filesystem-trigger 1000 1 > /dev/null
i=$[$i+1]
done

