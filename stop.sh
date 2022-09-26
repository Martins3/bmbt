#!/bin/sh

while true
do
  sleep 12
  for i in `ps -ef | grep "qemu" | grep -v grep | awk '{print $2}'`;
  do
    kill -9 $i;
  done
done
