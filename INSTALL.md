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

See Dockerfile in the root of this project on how this is setup.
Prerequisites:
- Docker

Create your build environment/image with docker:
```bash
# Build the image with DJGPP for cross compilation
sudo docker build ./ -t djgpp-adplay

# Run the image with your local working directory as code mounted in volume
sudo docker run --volume $(pwd):/build/adplay-dos -it djgpp-adplay
```
Inside the container:
```bash
# Switch to mounted build directory
cd /build/adplay-dos/build
# Make scripts executable
chmod +x *.sh
# Run build.sh script to compile everything
./build.sh
```