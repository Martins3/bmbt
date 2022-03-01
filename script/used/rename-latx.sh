#!/bin/bash

# originally latx's files use '-' to separate fiels, replace it with '_' later.
# In the second time sync, before substitute the file, change the files name
# style firstly.
dir=/home/maritns3/core/5000/core/bmbt/src/i386/LATX

shopt -s globstar
for file in "${dir}"/**; do
  if [[ -f $file ]]; then
    new_name="${file//_/-}"
    if [[ $file != "$new_name" ]];then
      mv "$file" "$new_name"
    fi
  fi
done
shopt -u globstar
