#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2023 MuseScore Limited
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

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --arch) PACKARCH="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

BUILD_TOOLS=$HOME/build_tools
ENV_FILE=$BUILD_TOOLS/environment.sh

mkdir -p $BUILD_TOOLS

# Let's remove the file with environment variables to recreate it
rm -f $ENV_FILE

echo "echo 'Setup MuseScore build environment'" >> $ENV_FILE

##########################################################################
# GET DEPENDENCIES
##########################################################################

# DISTRIBUTION PACKAGES

apt_packages=(
  cimg-dev
  curl
  desktop-file-utils
  file
  fuse
  git
  gpg
  libboost-dev
  libboost-filesystem-dev
  libboost-regex-dev
  libcairo2-dev
  libfuse-dev
  libtool
  libssl-dev
  patchelf
  software-properties-common # installs `add-apt-repository`
  unzip
  wget
  xxd
  p7zip-full
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
  zlib1g-dev
  make
  patch
  coreutils
  gawk
  sed
  desktop-file-utils # installs `desktop-file-validate` for appimagetool
  zsync # installs `zsyncmake` for appimagetool
  libgpgme-dev # install for appimagetool
  libglib2.0-dev
  librsvg2-dev
  argagg-dev
  libgcrypt20-dev
  libcurl4-openssl-dev
  libgpg-error-dev
  )

# MuseScore compiles without these but won't run without them
apt_packages_runtime=(
  libcups2
  libdbus-1-3
  libegl1-mesa-dev
  libgles2-mesa-dev
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

apt-get update # no package lists in Docker image
DEBIAN_FRONTEND="noninteractive" TZ="Europe/London" apt-get install -y --no-install-recommends \
  "${apt_packages[@]}" \
  "${apt_packages_runtime[@]}" \
  "${apt_packages_ffmpeg[@]}"

# add an exception for the "detected dubious ownership in repository" (only seen inside a Docker image)
git config --global --add safe.directory /MuseScore

##########################################################################
# GET QT
##########################################################################

apt_packages_qt6=(
  qt6-base-dev
  qt6-declarative-dev
  qt6-base-private-dev
  libqt6networkauth6-dev
  libqt6qml6
  qml6-module-* # installs all qml modules
  libqt6quick6
  libqt6quickcontrols2-6
  libqt6quicktemplates2-6
  libqt6quickwidgets6
  libqt6xml6
  libqt6svg6-dev
  qt6-tools-dev
  qt6-tools-dev-tools
  libqt6printsupport6
  libqt6opengl6-dev
  qt6-l10n-tools
  libqt6core5compat6-dev
  qt6-scxml-dev
  qt6-wayland
  libqt6websockets6-dev
  )

apt-get install -y \
  "${apt_packages_qt6[@]}"

case $PACKARCH in
  aarch64)
    qt_dir="/usr/lib/aarch64-linux-gnu/qt6"
    ;;
  armv7l)
    qt_dir="/usr/lib/arm-linux-gnueabihf/qt6"
    ;;
  *)
    echo "Unknown architecture: $PACKARCH"
    exit 1
    ;;
esac

if [[ ! -d "${qt_dir}" ]]; then
  echo "Qt directory not found: ${qt_dir}"
  exit 1
fi

# https://askubuntu.com/questions/1460242/ubuntu-22-04-with-qt6-qmake-could-not-find-a-qt-installation-of
qtchooser -install qt6 $(which qmake6)

##########################################################################
# GET TOOLS
##########################################################################

# COMPILER
apt_packages_compiler=(
  automake
  gcc
  g++
  )

apt-get install -y --no-install-recommends \
  "${apt_packages_compiler[@]}"

# CMAKE
# Get newer CMake (only used cached version if it is the same)
apt-get install -y --no-install-recommends cmake
cmake --version

# Ninja
apt-get install -y --no-install-recommends ninja-build
echo "ninja version"
ninja --version

echo export QT_PATH="${qt_dir}" >> ${ENV_FILE}
echo export QT_PLUGIN_PATH="${qt_dir}/plugins" >> ${ENV_FILE}
echo export QML2_IMPORT_PATH="${qt_dir}/qml" >> ${ENV_FILE}
echo export CFLAGS="-Wno-psabi" >> ${ENV_FILE}
echo export CXXFLAGS="-Wno-psabi" >> ${ENV_FILE}
# explicitly set QMAKE path for linuxdeploy-plugin-qt
echo export QMAKE="/usr/bin/qmake6" >> ${ENV_FILE}

##########################################################################
# POST INSTALL
##########################################################################

chmod +x "$ENV_FILE"

# # tidy up (reduce size of Docker image)
# apt-get clean autoclean
# apt-get autoremove --purge -y
# rm -rf /tmp/* /var/{cache,log,backups}/* /var/lib/apt/*

df -h .
echo "Setup script done"
