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
echo "Setup MacOS build environment"

trap 'echo Setup failed; exit 1' ERR
SKIP_ERR_FLAG=true

export MACOSX_DEPLOYMENT_TARGET=10.14

# install dependencies
wget -c --no-check-certificate -nv -O bottles.zip https://musescore.org/sites/musescore.org/files/2020-02/bottles-MuseScore-3.0-yosemite.zip
unzip bottles.zip

# we don't use freetype
rm bottles/freetype* | $SKIP_ERR_FLAG

brew update >/dev/null | $SKIP_ERR_FLAG

# additional dependencies
#brew install jack

BREW_CELLAR=$(brew --cellar)
BREW_PREFIX=$(brew --prefix)

function fixBrewPath {
  DYLIB_FILE=$1
  BREW_CELLAR=$(brew --cellar)
  BREW_PREFIX=$(brew --prefix)
  chmod 644 $DYLIB_FILE
  # change ID
  DYLIB_ID=$(otool -D  $DYLIB_FILE | tail -n 1)
  if [[ "$DYLIB_ID" == *@@HOMEBREW_CELLAR@@* ]]
  then
      PSLASH=$(echo $DYLIB_ID | sed "s,@@HOMEBREW_CELLAR@@,$BREW_CELLAR,g")
      install_name_tool -id $PSLASH $DYLIB_FILE
  fi
  if [[ "$DYLIB_ID" == *@@HOMEBREW_PREFIX@@* ]]
  then
      PSLASH=$(echo $DYLIB_ID | sed "s,@@HOMEBREW_PREFIX@@,$BREW_PREFIX,g")
      install_name_tool -id $PSLASH $DYLIB_FILE
  fi
  # Change dependencies
  for P in `otool -L $DYLIB_FILE | awk '{print $1}'`
  do
    if [[ "$P" == *@@HOMEBREW_CELLAR@@* ]]
    then
        PSLASH=$(echo $P | sed "s,@@HOMEBREW_CELLAR@@,$BREW_CELLAR,g")
        install_name_tool -change $P $PSLASH $DYLIB_FILE
    fi
    if [[ "$P" == *@@HOMEBREW_PREFIX@@* ]]
    then
        PSLASH=$(echo $P | sed "s,@@HOMEBREW_PREFIX@@,$BREW_PREFIX,g")
        install_name_tool -change $P $PSLASH $DYLIB_FILE
    fi
  done
  chmod 444 $DYLIB_FILE
}
export -f fixBrewPath

function installBottleManually {
  brew unlink $1
  rm -rf /usr/local/Cellar/$1
  tar xzvf bottles/$1*.tar.gz -C $BREW_CELLAR
  find $BREW_CELLAR/$1 -type f -name '*.pc' -exec sed -i '' "s:@@HOMEBREW_CELLAR@@:$BREW_CELLAR:g" {} +
  find $BREW_CELLAR/$1 -type f -name '*.dylib' -exec bash -c 'fixBrewPath "$1"' _ {} \;
  brew link $1
}

installBottleManually libogg
installBottleManually libvorbis
installBottleManually flac
installBottleManually libsndfile

# fixing install python 3.9 error (it is a dependency for ninja)
rm '/usr/local/bin/2to3'
brew install ninja pkg-config

# Qt
export QT_SHORT_VERSION=5.15.9
export QT_PATH=$HOME/Qt
export QT_MACOS=$QT_PATH/$QT_SHORT_VERSION/clang_64
export PATH=$PATH:$QT_MACOS/bin
echo "PATH=$PATH" >> $GITHUB_ENV
wget -nv -O qt5.zip https://s3.amazonaws.com/utils.musescore.org/Qt5159_mac.zip
mkdir -p $QT_MACOS
unzip -qq qt5.zip -d $QT_MACOS
rm qt5.zip

# Dump syms
wget -q --show-progress -O dump_syms.7z "https://s3.amazonaws.com/utils.musescore.org/breakpad/macos/x86-64/dump_syms.7z"
7z x -y dump_syms.7z -o"$HOME/breakpad"

# VST SDK
wget -q --show-progress -O vst_sdk.7z "https://s3.amazonaws.com/utils.musescore.org/VST3_SDK_37.7z"
7z x -y vst_sdk.7z -o"$HOME/vst"
echo "VST3_SDK_PATH=$HOME/vst/VST3_SDK" >> $GITHUB_ENV

#install sparkle
export SPARKLE_VERSION=1.20.0
mkdir Sparkle-${SPARKLE_VERSION}
cd Sparkle-${SPARKLE_VERSION}
wget -nv https://github.com/sparkle-project/Sparkle/releases/download/${SPARKLE_VERSION}/Sparkle-${SPARKLE_VERSION}.tar.bz2
tar jxf Sparkle-${SPARKLE_VERSION}.tar.bz2
cd ..
mkdir -p ~/Library/Frameworks
mv Sparkle-${SPARKLE_VERSION}/Sparkle.framework ~/Library/Frameworks/
rm -rf Sparkle-${SPARKLE_VERSION}

echo "Setup script done"
