#!/bin/bash
# You can also do a clean build from git instead of code copied into Docker container
# git clone https://github.com/adplug/adplay-dos.git --branch dos-1.6 --single-branch
cd /build/adplay-dos
#build window
cd window
# Only needed if cloning from older git repo (<=adplay 1.6)
# cp Makefile.gcc Makefile
make CC=gcc prefix=/djgpp
make install CC=gcc prefix=/djgpp

cd ..

# Only needed if cloning from older git repo (<=adplay 1.6)
# cp Makefile.gcc Makefile
#changes to Makefile
#prefix = /djgpp
#iprefix = /djgpp
#CPPFLAGS += -I/djgpp/include/libbinio -I/djgpp/include -I/djgpp/include/adplug
#CXXFLAGS += -I/djgpp/include/libbinio -I/djgpp/include -I/djgpp/include/adplug

#iprefix = $(HOME)/djgpp
#LDFLAGS += -lstdc++ -lwindow -ladplug -lbinio -lgcctimer -L/djgpp/lib
#LDFLAGS += -L/djgpp/lib -L/djgpp/lib/gcc/i586-pc-msdosdjgpp/12.2.0

make binarydist