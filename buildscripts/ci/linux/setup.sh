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
PACKARCH="x86_64" # x86_64, armv7l, aarch64
COMPILER="gcc" # gcc, clang

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
  libasound2-dev 
  libfontconfig1-dev
  libfreetype6
  libfreetype6-dev
  libgcrypt20-dev
  libgl1-mesa-dev
  libglib2.0-dev
  libgpgme-dev # install for appimagetool
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
  libcups2
  libdbus-1-3
  libegl1-mesa-dev
  libgles2-mesa-dev
  libodbc1
  libodbc2
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
  # gstreamer for QtMultimedia - Shouldn't be in the bundle at the moment
  # libunwind-dev
  # libgstreamer1.0-dev
  # libgstreamer-plugins-base1.0-dev
  # libgstreamer-plugins-bad1.0-dev 
  # gstreamer1.0-plugins-base 
  # gstreamer1.0-plugins-good 
  # gstreamer1.0-plugins-bad 
  # gstreamer1.0-plugins-ugly 
  # gstreamer1.0-libav 
  # gstreamer1.0-tools 
  # gstreamer1.0-x 
  # gstreamer1.0-alsa 
  # gstreamer1.0-gl 
  # gstreamer1.0-gtk3  
  # gstreamer1.0-pulseaudio
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
# GET QT
##########################################################################

case "$PACKARCH" in
  x86_64)
    # Get newer Qt (only used cached version if it is the same)
    qt_version="624"
    # qt_revision="r2" # added websocket module
    qt_revision="r3" # added multimedia module
    qt_dir="$BUILD_TOOLS/Qt/${qt_version}"
    if [[ ! -d "${qt_dir}" ]]; then
      mkdir -p "${qt_dir}"
      qt_url="https://s3.amazonaws.com/utils.musescore.org/Qt${qt_version}_gcc64_${qt_revision}.7z"
      wget -q --show-progress -O qt.7z "${qt_url}"
      7z x -y qt.7z -o"${qt_dir}"
      rm qt.7z
    fi

    echo export PATH="${qt_dir}/bin:\${PATH}" >> ${ENV_FILE}
    echo export LD_LIBRARY_PATH="${qt_dir}/lib:\${LD_LIBRARY_PATH}" >> ${ENV_FILE}
    echo export QT_PATH="${qt_dir}" >> ${ENV_FILE}
    echo export QT_PLUGIN_PATH="${qt_dir}/plugins" >> ${ENV_FILE}
    echo export QML2_IMPORT_PATH="${qt_dir}/qml" >> ${ENV_FILE}
    ;;
  armv7l | aarch64)
    apt_packages_qt6=(
      libqt6core5compat6-dev
      libqt6networkauth6-dev
      libqt6opengl6-dev
      libqt6printsupport6
      libqt6qml6
      libqt6quick6
      libqt6quickcontrols2-6
      libqt6quicktemplates2-6
      libqt6quickwidgets6
      libqt6svg6-dev
      libqt6websockets6-dev
      libqt6xml6
      qml6-module-* # installs all qml modules
      qt6-base-dev
      qt6-base-private-dev
      qt6-declarative-dev
      qt6-gtk-platformtheme
      qt6-l10n-tools
      qt6-scxml-dev
      qt6-tools-dev
      qt6-tools-dev-tools
      qt6-wayland
      )

    $SUDO apt-get install -y --no-install-recommends \
      "${apt_packages_qt6[@]}"

    case $PACKARCH in
      aarch64)
        qt_dir="/usr/lib/aarch64-linux-gnu/qt6"
        ;;
      armv7l)
        qt_dir="/usr/lib/arm-linux-gnueabihf/qt6"
        ;;
    esac

    if [[ ! -d "${qt_dir}" ]]; then
      echo "Qt directory not found: ${qt_dir}"
      exit 1
    fi

    export QT_SELECT=qt6
    echo export QT_SELECT=qt6 >> ${ENV_FILE}
    echo export QT_PATH="${qt_dir}" >> ${ENV_FILE}
    echo export QT_PLUGIN_PATH="${qt_dir}/plugins" >> ${ENV_FILE}
    echo export QML2_IMPORT_PATH="${qt_dir}/qml" >> ${ENV_FILE}
    echo export CFLAGS="-Wno-psabi" >> ${ENV_FILE}
    echo export CXXFLAGS="-Wno-psabi" >> ${ENV_FILE}
    # explicitly set QMAKE path for linuxdeploy-plugin-qt
    echo export QMAKE="/usr/bin/qmake6" >> ${ENV_FILE}

    # https://askubuntu.com/questions/1460242/ubuntu-22-04-with-qt6-qmake-could-not-find-a-qt-installation-of
    qtchooser -install qt6 $(which qmake6)
    ;;

  *)
    echo "Unknown architecture: $PACKARCH"
    exit 1
    ;;
esac

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
  x86_64)
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
  x86_64)
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

##########################################################################
# POST INSTALL
##########################################################################

chmod +x "$ENV_FILE"

if [[ "$PACKARCH" == "armv7l" ]]; then
  # add an exception for the "detected dubious ownership in repository" (only seen inside a Docker image)
  git config --global --add safe.directory /MuseScore
fi

df -h .
echo "Setup script done"
