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

# For maximum AppImage compatibility, build on the oldest Linux distribution
# that still receives security updates from its manufacturer.

echo "Setup Linux build environment"
trap 'echo Setup failed; exit 1' ERR

df -h .

BUILD_TOOLS=$HOME/build_tools
ENV_FILE=$BUILD_TOOLS/environment.sh

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

sudo apt-get install -y --no-install-recommends "${apt_packages[@]}"


##########################################################################
# GET TOOLS
##########################################################################

# COMPILER

gcc_version="10"
sudo apt-get install -y --no-install-recommends "g++-${gcc_version}"
sudo update-alternatives \
  --install /usr/bin/gcc gcc "/usr/bin/gcc-${gcc_version}" 40 \
  --slave /usr/bin/g++ g++ "/usr/bin/g++-${gcc_version}"

echo export CC="/usr/bin/gcc-${gcc_version}" >> ${ENV_FILE}
echo export CXX="/usr/bin/g++-${gcc_version}" >> ${ENV_FILE}

gcc-${gcc_version} --version
g++-${gcc_version} --version 

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
