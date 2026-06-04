#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited and others
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

# For maximum AppImage compatibility, build on the oldest Linux distribution
# that still receives security updates from its manufacturer.

echo "Setup Linux build environment"
trap 'echo Setup failed; exit 1' ERR

df -h .

BUILD_TOOLS=$HOME/build_tools
ENV_FILE=$BUILD_TOOLS/environment.sh
COMPILER="clang" # gcc, clang

mkdir -p $BUILD_TOOLS

# Let's remove the file with environment variables to recreate it
rm -f $ENV_FILE

echo "echo 'Setup MuseScore build environment'" >> $ENV_FILE

##########################################################################
# GET DEPENDENCIES
##########################################################################

apt_packages=(
  # Alphabetical order please!
  curl
  p7zip-full
  unzip
  wget
  )

sudo apt install -y --no-install-recommends "${apt_packages[@]}"


##########################################################################
# GET TOOLS
##########################################################################

# COMPILER
if [ "$COMPILER" == "gcc" ]; then

  gcc_version="14"
  sudo apt install -y --no-install-recommends "g++-${gcc_version}"

  for alt in gcc g++; do
    if update-alternatives --query "$alt" >/dev/null 2>&1; then
      sudo update-alternatives --remove-all "$alt"
    fi
    sudo update-alternatives --install "/usr/bin/$alt" "$alt" "/usr/bin/${alt}-${gcc_version}" 100
  done

  echo export CC="/usr/bin/gcc" >> "${ENV_FILE}"
  echo export CXX="/usr/bin/g++" >> "${ENV_FILE}"

  gcc --version
  g++ --version

elif [ "$COMPILER" == "clang" ]; then

  clang_version="20"
  sudo apt install -y --no-install-recommends "clang-${clang_version}"
  sudo apt install -y --no-install-recommends "clang-tools-${clang_version}"

  for alt in clang clang++ clang-scan-deps; do
    if update-alternatives --query "$alt" >/dev/null 2>&1; then
      sudo update-alternatives --remove-all "$alt"
    fi
    sudo update-alternatives --install "/usr/bin/$alt" "$alt" "/usr/bin/${alt}-${clang_version}" 100
  done

  echo export CC="/usr/bin/clang" >> "${ENV_FILE}"
  echo export CXX="/usr/bin/clang++" >> "${ENV_FILE}"

  clang --version
  clang++ --version

else
  echo "Unknown compiler: $COMPILER"
fi

# CMake
echo "cmake version"
cmake --version

# Ninja
echo "ninja version"
ninja --version

##########################################################################
# POST INSTALL
##########################################################################

chmod +x "$ENV_FILE"

echo "Setup script done"
