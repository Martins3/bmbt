#!/bin/bash

compiledb make
sed -i 's/loongson\/core\/bmbt/maritns3\/core\/5000\/core\/bmbt/g' compile_commands.json
