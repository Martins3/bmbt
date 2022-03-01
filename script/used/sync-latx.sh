#!/bin/bash

location=~/core/5000/core
LATX_PROJECT=${location}/lat
BMBT_PROJECT=${location}/bmbt

LATX=${LATX_PROJECT}/target/i386/latx
LOC=${BMBT_PROJECT}/src/i386/LATX

ag CONFIG_LATX ${LATX} > latx_patch_for_qemu.txt
ag SYNC_LATX ${LOC} > bmbt_patch_for_latx.txt
