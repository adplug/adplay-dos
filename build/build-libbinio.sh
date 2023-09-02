#!/bin/bash
#Libbinio
cd /build/
git clone https://github.com/adplug/libbinio.git --branch libbinio-1.5 --single-branch
cd libbinio
autoreconf --install
autoupdate
libtoolize --force
aclocal
autoconf
automake --add-missing
./configure --enable-maintainer-mode --host=i386-pc-msdosdjgpp --prefix=/djgpp
make install
# # /djgpp/lib/libbinio.la needs to be patched, for some reason dependency_libs is not set correctly and is prefixed with /usr/local
# patch copied libbinio.la in /djgpp/lib to point to correct stdc++ path
# Not sure why this is needed, could not find a way to correct this from configure/build
# Changes:
# dependency_libs=' /usr/local/djgpp/lib/gcc/i586-pc-msdosdjgpp/12.2.0/libstdc++.la'
# To:
# dependency_libs=' /djgpp/lib/gcc/i586-pc-msdosdjgpp/12.2.0/libstdc++.la'

cp -v /build/adplay-dos/build/libbinio-patches/libbinio.la /djgpp/lib/
cd ..