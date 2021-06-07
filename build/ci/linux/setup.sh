#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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

# Go one-up from MuseScore root dir regardless of where script was run from:
cd "$(dirname "$(readlink -f "${0}")")/../../../.."

# Let's remove the file with environment variables to recreate it
ENV_FILE=./musescore_environment.sh
rm -f ${ENV_FILE}

echo "echo 'Setup MuseScore build environment'" >> ${ENV_FILE}

##########################################################################
# GET DEPENDENCIES
##########################################################################

# DISTRIBUTION PACKAGES

# These are installed by default on Travis CI, but not on Docker
apt_packages_basic=(
  # Alphabetical order please!
  file
  git
  pkg-config
  software-properties-common # installs `add-apt-repository`
  unzip
  p7zip-full
  )

# These are the same as on Travis CI
apt_packages_standard=(
  # Alphabetical order please!
  curl
  libasound2-dev 
  libfontconfig1-dev
  libfreetype6-dev
  libfreetype6
  libgl1-mesa-dev
  libjack-dev
  libnss3-dev
  libportmidi-dev
  libpulse-dev
  libsndfile1-dev
  make
  wget
  )

# MuseScore compiles without these but won't run without them
apt_packages_runtime=(
  # Alphabetical order please!
  libcups2
  libdbus-1-3
  libegl1-mesa-dev
  libodbc1
  libpq-dev
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
  libxcb-render-util0
  libxcb-xinerama0
  )

apt-get update # no package lists in Docker image
apt-get install -y --no-install-recommends \
  "${apt_packages_basic[@]}" \
  "${apt_packages_standard[@]}" \
  "${apt_packages_runtime[@]}"

##########################################################################
# GET QT
##########################################################################

# Get newer Qt (only used cached version if it is the same)
qt_version="5152"
qt_dir="Qt/${qt_version}"
if [[ ! -d "${qt_dir}" ]]; then
  mkdir -p "${qt_dir}"
  qt_url="https://s3.amazonaws.com/utils.musescore.org/Qt${qt_version}_gcc64.7z"
  wget -q --show-progress -O qt5.7z "${qt_url}"
  7z x -y qt5.7z -o"${qt_dir}"
fi
qt_path="${PWD%/}/${qt_dir}"

echo export PATH="${qt_path}/bin:\${PATH}" >> ${ENV_FILE}
echo export LD_LIBRARY_PATH="${qt_path}/lib:\${LD_LIBRARY_PATH}" >> ${ENV_FILE}
echo export QT_PATH="${qt_path}" >> ${ENV_FILE}
echo export QT_PLUGIN_PATH="${qt_path}/plugins" >> ${ENV_FILE}
echo export QML2_IMPORT_PATH="${qt_path}/qml" >> ${ENV_FILE}


##########################################################################
# GET TOOLS
##########################################################################

# COMPILER

gcc_version="7"
apt-get install -y --no-install-recommends "g++-${gcc_version}"
update-alternatives \
  --install /usr/bin/gcc gcc "/usr/bin/gcc-${gcc_version}" 40 \
  --slave /usr/bin/g++ g++ "/usr/bin/g++-${gcc_version}"

echo export CC="/usr/bin/gcc-${gcc_version}" >> ${ENV_FILE}
echo export CXX="/usr/bin/g++-${gcc_version}" >> ${ENV_FILE}

gcc-${gcc_version} --version
g++-${gcc_version} --version 

# CMAKE
# Get newer CMake (only used cached version if it is the same)
cmake_version="3.16.0"
cmake_dir="cmake/${cmake_version}"
if [[ ! -d "${cmake_dir}" ]]; then
  mkdir -p "${cmake_dir}"
  cmake_url="https://cmake.org/files/v${cmake_version%.*}/cmake-${cmake_version}-Linux-x86_64.tar.gz"
  wget -q --show-progress --no-check-certificate -O - "${cmake_url}" | tar --strip-components=1 -xz -C "${cmake_dir}"
fi
echo export PATH="${PWD%/}/${cmake_dir}/bin:\${PATH}" >> ${ENV_FILE}
export PATH="${PWD%/}/${cmake_dir}/bin:${PATH}"
cmake --version

# Ninja
echo "Get Ninja"
mkdir -p $HOME/build_tools/Ninja
wget -q --show-progress -O $HOME/build_tools/Ninja/ninja "https://s3.amazonaws.com/utils.musescore.org/build_tools/linux/Ninja/ninja"
chmod +x $HOME/build_tools/Ninja/ninja
echo export PATH="$HOME/build_tools/Ninja:\${PATH}" >> ${ENV_FILE}
echo "ninja version"
$HOME/build_tools/Ninja/ninja --version

# Dump syms
wget -q --show-progress -O dump_syms.7z "https://s3.amazonaws.com/utils.musescore.org/breakpad/linux/x86-64/dump_syms.7z"
7z x -y dump_syms.7z -o"$HOME/breakpad"
echo export DUMPSYMS_BIN="$HOME/breakpad/dump_syms" >> $ENV_FILE

##########################################################################
# OTHER
##########################################################################
wget -q --show-progress -O vst_sdk.7z "https://s3.amazonaws.com/utils.musescore.org/VST3_SDK_37.7z"
7z x -y vst_sdk.7z -o"$HOME/vst"
echo export VST3_SDK_PATH="$HOME/vst/VST3_SDK" >> $ENV_FILE

##########################################################################
# POST INSTALL
##########################################################################

chmod +x "${ENV_FILE}"

# # tidy up (reduce size of Docker image)
# apt-get clean autoclean
# apt-get autoremove --purge -y
# rm -rf /tmp/* /var/{cache,log,backups}/* /var/lib/apt/*

df -h .
echo "Setup script done"

