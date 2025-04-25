#!/bin/bash -eu

# This script is called by the oss-fuzz main project when compiling the fuzz
# targets. This script is regression tested by ci_oss.sh.

# Save off the current folder as the build root.
export BUILD_ROOT=$PWD

echo "CC: ${CC:-}"
echo "CXX: ${CXX:-}"
echo "LIB_FUZZING_ENGINE: ${LIB_FUZZING_ENGINE:-}"
echo "CFLAGS: ${CFLAGS:-}"
echo "CXXFLAGS: ${CXXFLAGS:-}"
echo "OUT: ${OUT:-}"

export MAKEFLAGS+="-j$(nproc)"

# Install dependencies
apt-get -y install autoconf autogen automake libtool pkg-config python

# For now, do not install the following libraries (as they won't be in the
# final image):
# libasound2-dev libflac-dev libogg-dev libopus-dev libvorbis-dev

# Compile the fuzzer.
./autogen.sh
./configure --disable-shared --enable-ossfuzzers
make V=1

# Copy the fuzzer to the output directory.
cp -v ossfuzz/sndfile_fuzzer $OUT/
