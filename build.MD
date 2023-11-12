# Building AdPlay/DOS

## AdPlay/DOS Build Requirements
To compile AdPlay/DOS from source, you need [DJGPP](http://www.delorie.com/djgpp/).

Alternatively you can download ready to be used binaries from [build-djgpp](https://github.com/andrewwutw/build-djgpp/releases).

Currently DJGPP 2.01 is used, together with GCC 12.2.0. See [GitHub workflow](.github/workflows) for latest versions used to be sure.

Also the following packages/binaries have to be installed:
```
# Example for Ubuntu
sudo apt install texlive-latex-base texinfo libfl2 libfl-dev bison flex curl make texinfo zlib1g-dev unzip wget bzip2 zip git bash automake libtool pkg-config upx
```

## Dependencies
AdPlay/DOS depends on the following libraries:

- [libbinio](https://github.com/adplug/libbinio) >= 1.5 (older versions might also work, but untested)
- [AdPlug](https://github.com/adplug/adplug) > 2.3.3 (note: older Adplay versions <= 1.6 relied on Adplug >= 1.1 <= 1.5)
- [DJGPP Timer Library](https://github.com/adplug/gcctimer) >= 1.0
- Portable Textmode Window Class Library >= 1.1 (part of adplay-dos, in ./window/)

Also see the [GitHub workflow](.github/workflows) for latest dependencies.

## Build Instructions
1. Make sure the dependencies mentioned above are compiled and installed. Be extra carefull to ensure they are compiled using DJGPP GCC (DOS), and not the regular GCC (Linux) that might be installed on your system. If you are not sure you can run `which gcc` to make sure it's pointing to DJGPP.

1. Use Makefile to build:
```bash
# Set gcc_version to your DJGPP GCC version used
make gcc_version=12.2.0 binarydist
# The 'binarydist' Makefile target will build a compressed binary distribution of AdPlay, zipped into adplay*.zip.
```

## Debug logging the AdPlug core library with AdPlay/DOS
If you compiled the AdPlug core library with debug logging support enabled,
AdPlay/DOS automatically redirects the output from the default `stderr` to a
file called `debug.log` in the directory, it was started from. This is to
prevent screen cluttering, which would otherwise occur when AdPlug is logging
directly to the console, on which AdPlay/DOS has its GUI output.

## Debug logging AdPlay/DOS
To enable debug logging, you have to define the **DEBUG** preprocessor
variable.

This instructs all source files to generate debug logging output. The output
goes to different logfiles by default. Please refer to the source code for
information on the file names.
