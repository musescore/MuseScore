#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

echo "Setup Linux build environment"
trap 'echo Setup failed; exit 1' ERR

df -h .

BUILD_TOOLS=$HOME/build_tools
ENV_FILE=$BUILD_TOOLS/environment.sh
PACKARCH="x86_64" # x86_64, armv7l, aarch64, wasm
COMPILER="gcc" # gcc, clang
EMSDK_VERSION="3.1.70" # for Qt 6.9

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --arch) PACKARCH="$2"; shift ;;
        --compiler) COMPILER="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

mkdir -p $BUILD_TOOLS

# Let's remove the file with environment variables to recreate it
rm -f $ENV_FILE

echo "echo 'Setup MuseScore build environment'" >> $ENV_FILE

if [[ "$PACKARCH" == "armv7l" ]]; then
  SUDO=""
  export DEBIAN_FRONTEND="noninteractive" TZ="Europe/London"
else
  SUDO="sudo"
fi

##########################################################################
# GET DEPENDENCIES
##########################################################################

apt_packages=(
  coreutils
  curl
  desktop-file-utils # installs `desktop-file-validate` for appimagetool
  gawk
  git
  lcov
  libasound2-dev
  libcups2-dev
  libfontconfig1-dev
  libfreetype6-dev
  libgcrypt20-dev
  libgl1-mesa-dev
  libglib2.0-dev
  libgpgme-dev # install for appimagetool
  libinstpatch-dev
  libjack-dev
  libnss3-dev
  libportmidi-dev
  libpulse-dev
  librsvg2-dev
  libsndfile1-dev
  libssl-dev
  libtool
  make
  p7zip-full
  sed
  software-properties-common # installs `add-apt-repository`
  unzip
  wget
  zsync # installs `zsyncmake` for appimagetool
  )

# MuseScore compiles without these but won't run without them
apt_packages_runtime=(
  # Alphabetical order please!
  libdbus-1-3
  libegl1-mesa-dev
  libgles2-mesa-dev
  libodbc2
  libpq-dev
  libssl-dev
  libxcomposite-dev
  libxcursor-dev
  libxi-dev
  libxkbcommon-x11-0
  libxrandr2
  libxtst-dev
  libdrm-dev
  libxcb-icccm4
  libxcb-image0
  libxcb-keysyms1
  libxcb-randr0
  libxcb-render-util0
  libxcb-xinerama0
  libxcb-xkb-dev
  libxkbcommon-dev
  libopengl-dev
  libvulkan-dev
  )

apt_packages_ffmpeg=(
  ffmpeg
  libavcodec-dev
  libavformat-dev
  libswscale-dev
  )

$SUDO apt-get update
$SUDO apt-get install -y --no-install-recommends \
  "${apt_packages[@]}" \
  "${apt_packages_runtime[@]}" \
  "${apt_packages_ffmpeg[@]}"

##########################################################################
# GET TOOLS
##########################################################################

# COMPILER
if [ "$COMPILER" == "gcc" ]; then

  gcc_version="10"
  $SUDO apt-get install -y --no-install-recommends "g++-${gcc_version}"
  $SUDO update-alternatives \
    --install /usr/bin/gcc gcc "/usr/bin/gcc-${gcc_version}" 40 \
    --slave /usr/bin/g++ g++ "/usr/bin/g++-${gcc_version}"

  echo export CC="/usr/bin/gcc-${gcc_version}" >> ${ENV_FILE}
  echo export CXX="/usr/bin/g++-${gcc_version}" >> ${ENV_FILE}

  gcc-${gcc_version} --version
  g++-${gcc_version} --version

elif [ "$COMPILER" == "clang" ]; then

  $SUDO apt-get install clang
  echo export CC="/usr/bin/clang" >> ${ENV_FILE}
  echo export CXX="/usr/bin/clang++" >> ${ENV_FILE}

  clang --version
  clang++ --version

else
  echo "Unknown compiler: $COMPILER"
fi

# CMAKE
# Get newer CMake (only used cached version if it is the same)
case "$PACKARCH" in
  x86_64 | wasm)
    cmake_version="3.24.0"
    cmake_dir="$BUILD_TOOLS/cmake/${cmake_version}"
    if [[ ! -d "$cmake_dir" ]]; then
      mkdir -p "$cmake_dir"
      cmake_url="https://cmake.org/files/v${cmake_version%.*}/cmake-${cmake_version}-linux-x86_64.tar.gz"
      wget -q --show-progress --no-check-certificate -O - "${cmake_url}" | tar --strip-components=1 -xz -C "${cmake_dir}"
    fi
    export PATH="$cmake_dir/bin:$PATH"
    echo export PATH="$cmake_dir/bin:\${PATH}" >> ${ENV_FILE}
    ;;
  armv7l | aarch64)
    $SUDO apt-get install -y --no-install-recommends cmake
    ;;
esac
cmake --version

# Ninja
case "$PACKARCH" in
  x86_64 | wasm)
    echo "Get Ninja"
    ninja_dir=$BUILD_TOOLS/Ninja
    if [[ ! -d "$ninja_dir" ]]; then
      mkdir -p $ninja_dir
      wget -q --show-progress -O $ninja_dir/ninja "https://s3.amazonaws.com/utils.musescore.org/build_tools/linux/Ninja/ninja"
      chmod +x $ninja_dir/ninja
    fi
    export PATH="${ninja_dir}:${PATH}"
    echo export PATH="${ninja_dir}:\${PATH}" >> ${ENV_FILE}
    ;;
  armv7l | aarch64)
    $SUDO apt-get install -y --no-install-recommends ninja-build
    ;;
esac
echo "ninja version"
ninja --version

if [[ "$PACKARCH" == "wasm" ]]; then
  git clone https://github.com/emscripten-core/emsdk.git $BUILD_TOOLS/emsdk
  origin_dir=$(pwd)
  cd $BUILD_TOOLS/emsdk
  git pull
  ./emsdk install $EMSDK_VERSION
  ./emsdk activate $EMSDK_VERSION
  echo "source $BUILD_TOOLS/emsdk/emsdk_env.sh" >> ${ENV_FILE}
  cd $origin_dir
fi

##########################################################################
# POST INSTALL
##########################################################################

chmod +x "$ENV_FILE"

df -h .
echo "Setup script done"
