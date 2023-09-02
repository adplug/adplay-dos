#!/bin/bash
source /djgpp/setenv

cd /build/
./build-libbinio.sh
./build-gcctimer.sh
./build-adplug.sh
./build-adplay-dos.sh