FROM ubuntu:22.04
RUN apt update
RUN apt upgrade -y
RUN apt install -y bison flex curl make texinfo zlib1g-dev unzip wget bzip2 git bash automake libtool pkg-config

RUN wget https://github.com/andrewwutw/build-djgpp/releases/download/v3.4/djgpp-linux64-gcc1220.tar.bz2
RUN bunzip2 djgpp-linux64-gcc1220.tar.bz2
RUN tar -xvf djgpp-linux64-gcc1220.tar
RUN mkdir /build
RUN cd /build
COPY *.sh /build
RUN chmod +x /build/*.sh