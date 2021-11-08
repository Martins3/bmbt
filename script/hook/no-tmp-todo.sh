#!/bin/bash

if git diff --cached | grep TMP_TODO; then
    echo "there something todo !"
    exit 1
fi
exit 0
