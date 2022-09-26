#!/bin/sh

j=$1
i=0

while [ $i -le $j ]
do
  make run HAMT=1 1>>hamt_debug3.md
  i=$(($i + 1))
done
