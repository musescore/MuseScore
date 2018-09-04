#!/bin/bash (This file should be sourced rather than executed.)
# Sourcing allows environment variables to be used outside the script, but other
# changes also persist. (If you change directory then remember to change back!)

if [ "$(basename "$0")" == "environment.sh" ]; then
  echo "$0 was executed but it should have been sourced."
  exit 1
fi

initial_dir="${PWD}"
set -e # exit on error
set -x # echo commands

# Set compilers
export CC=/usr/bin/gcc-4.9
export CXX=/usr/bin/g++-4.9

#install cmake
CMAKE_URL="https://cmake.org/files/v3.10/cmake-3.10.1-Linux-x86_64.tar.gz"
mkdir cmake && travis_retry wget --no-check-certificate --quiet -O - ${CMAKE_URL} | tar --strip-components=1 -xz -C cmake
export PATH=${PWD}/cmake/bin:${PATH}

# Get Qt
mkdir qt5 && travis_retry wget -q -O qt5.zip http://utils.musescore.org.s3.amazonaws.com/qt593.zip
unzip -qq qt5.zip -d qt5
export PATH="${PWD}/qt5/bin:$PATH"
export QT_PLUGIN_PATH="${PWD}/qt5/plugins"
export QML2_IMPORT_PATH="${PWD}/qt5/qml"

# Setup install destination
mkdir "$HOME/software"
export PATH="$PATH:$HOME/bin:$HOME/software/bin"

# Prepare for post-install upload of artifacts to S3
export TRAVIS_SHORT_COMMIT="$(echo $TRAVIS_COMMIT | cut -c 1-8)"
export ARTIFACTS_TARGET_PATHS="$TRAVIS_SHORT_COMMIT"
export ARTIFACTS_PERMISSIONS=public-read
export TRAVIS_BUILD_DIR=vtest/html
#compatibility between ruby travis artifact and GO one
export ARTIFACTS_KEY=$ARTIFACTS_AWS_ACCESS_KEY_ID
export ARTIFACTS_SECRET=$ARTIFACTS_AWS_SECRET_ACCESS_KEY
artifacts -v || curl -sL https://raw.githubusercontent.com/meatballhat/artifacts/master/install
artifacts -v || curl -sL https://raw.githubusercontent.com/meatballhat/artifacts/master/install | bash

# IMPORTANT: Must now return shell to it's initial state:
set +x
set +e
cd "${initial_dir}"
