#!/bin/bash

PROJECT_DIR=$(git rev-parse --show-toplevel)
cd "$PROJECT_DIR" || exit 1
make -j10

if make -j10; then
    exit 1
fi
