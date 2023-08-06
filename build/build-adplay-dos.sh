git clone https://github.com/adplug/adplay-dos.git --branch dos-1.6 --single-branch
cd adplay-dos

#build window
cd window
cp Makefile.gcc Makefile
make CC=gcc prefix=/djgpp
make install CC=gcc prefix=/djgpp

cd ..

cp Makefile.gcc Makefile
#change Makefile
prefix = ~/djgpp
iprefix = $(HOME)/djgpp
CPPFLAGS += -I$(iprefix)/include/libbinio -I$(iprefix)/include -I$(iprefix)/include/adplug
CXXFLAGS += -I$(iprefix)/include/libbinio -I$(iprefix)/include -I$(iprefix)/include/adplug

iprefix = $(HOME)/djgpp
LDFLAGS += -lstdc++ -lwindow -ladplug -lbinio -lgcctimer -L$(iprefix)/lib
LDFLAGS = -L/home/aranvink/djgpp/lib -L/home/aranvink/djgpp/lib/gcc/i586-pc-msdosdjgpp/12.2.0

#change PATH_MAX=256 to adplay.cpp, filewnd.h, filewnd.cpp

make