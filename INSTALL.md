# Build notes
- DJGPP is the only supported GCC architecture.

- Use Makefile to build.

- If you set the DEBUG Makefile variable to 1, AdPlay will be compiled
  in debug mode, as described in build.txt. No setting of preprocessor
  variables necessary.

- The 'binarydist' Makefile target will build a binary distribution of
  AdPlay. A ZIP archiving program is needed for this.

# DJGPP cross compilation steps
Build system used is Ubuntu 22.04, using GCC 12.2.0

See build/Dockerfile