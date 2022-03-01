#!/bin/bash

location=~/core/5000
LATX=${location}/core/lat/target/i386/latx
LOC=${location}/core/bmbt/src/i386/LATX

mv ${LOC} /tmp
cp -r ${LATX} ${LOC}
