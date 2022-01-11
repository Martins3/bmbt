#!/bin/bash
xclip -o -selection clipboard | xargs -i addr2line -p --exe=./build_loongson_kernel__/bmbt.bin --functions {}
