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

# DEPENDENCES
apt_packages_basic=(
  file
  software-properties-common
  p7zip-full
  unzip
  )

apt_packages_standard=(
  curl
  libasound2-dev 
  libfontconfig1-dev
  libfreetype6-dev
  libfreetype6
  libgl1-mesa-dev
  libinstpatch-dev
  libjack-dev
  libmp3lame-dev
  libnss3-dev
  libportmidi-dev
  libpulse-dev
  libsndfile1-dev
  portaudio19-dev
  wget
  )

apt_packages_runtime=(
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
  libxcb-randr0
  libxcb-render-util0
  libxcb-xinerama0
  xvfb
  )

apt_packages_ffmpeg=(
  ffmpeg
  libavcodec-dev 
  libavformat-dev 
  libswscale-dev
  )

apt-get update
apt-get install -y --no-install-recommends \
  "${apt_packages_basic[@]}" \
  "${apt_packages_standard[@]}" \
  "${apt_packages_runtime[@]}" \
  "${apt_packages_ffmpeg[@]}"

# DISTROS
HERE="$(cd "$(dirname "$0")" && pwd)"
bash $HERE/install_mu.sh

# Google Fonts installation
TEMP_DIR=$(mktemp -d)
FONTS_DIR="$HOME/.local/share/fonts"

echo "Downloading Google Fonts..."
ZIP_URL="https://github.com/google/fonts/archive/main.zip"
mkdir -p "$TEMP_DIR"
cd "$TEMP_DIR"
curl -L -o fonts.zip "$ZIP_URL"

echo "Unpacking Google Fonts..."
unzip -q fonts.zip
cd fonts-main

echo "Installing Google Fonts..."
mkdir -p "$FONTS_DIR"
find . -type f \( -iname "*.ttf" -o -iname "*.otf" \) -print0 | xargs -0 -r mv-t "$FONTS_DIR"

echo "Installing Fonts Cache..."
fc-cache -f -v

echo "Cleaning..."
rm -rf "$TEMP_DIR"
