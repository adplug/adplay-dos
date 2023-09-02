#!/bin/bash
#Gcc timer
git clone https://github.com/adplug/gcctimer.git # No specific tags or branches available
cd gcctimer
make CC=gcc prefix=/djgpp install
cd ..