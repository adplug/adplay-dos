#Libbinio
git clone https://github.com/adplug/libbinio.git --branch libbinio-1.5 --single-branch
cd libbinio
autoreconf --install
autoupdate
libtoolize --force && aclocal && autoconf && automake --add-missing && ./configure --enable-maintainer-mode --host=i386-pc-msdosdjgpp
make prefix=/djgpp
mkdir /destlibbinio
make install DESTDIR=/destlibbinio
cp -R /destlibbinio/usr/local/include /djgpp
cp -R /destlibbinio/usr/local/lib /djgpp
cp -R /destlibbinio/usr/local/share /djgpp
cd ..