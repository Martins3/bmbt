#!/bin/bash

git add -A

read -r -p "Enter msg: " fullname
msg="(code review): ${fullname}"
echo "$msg"

read -r -p "Continue? (Y/N): " confirm && [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]] || exit 1

git commit -m "$msg"
