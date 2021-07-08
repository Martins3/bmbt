#!/bin/bash

git add -A
DIR=/home/maritns3/core/ld/DuckBuBi
num=$(ag FIXME "${DIR}" | wc -l)
num=$((num - 2))
echo "$num"

read -r -p "Enter msg: " fullname
msg="(FIXME ${num}): ${fullname}"
echo "$msg"

read -r -p "Continue? (Y/N): " confirm && [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]] || exit 1

git commit -m "$msg"
