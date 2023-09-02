#!/bin/bash

# Build Gcc timer

cd /build/
git clone https://github.com/adplug/gcctimer.git # No specific tags or branches available
cd gcctimer
# For some reason we cannot call install straigth away
make CC=gcc prefix=/djgpp
make CC=gcc prefix=/djgpp install
cd ..