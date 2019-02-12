#!/bin/bash

export QT_SHORT_VERSION=5.12
export QT_LONG_VERSION=5.12.0
export QT_INSTALLER_ROOT=qt-opensource-mac-x64-clang-${QT_LONG_VERSION}
export QT_INSTALLER_FILENAME=${QT_INSTALLER_ROOT}.dmg

export QT_PATH=$HOME/qt
export QT_MACOS=$QT_PATH/$QT_SHORT_VERSION/clang_64
export PATH=$PATH:$QT_MACOS/bin

#set NIGHTLY_BUILD variable if MSCORE_UNSTABLE is TRUE in CMakeLists.txt
if [ "$(grep '^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)' CMakeLists.txt)" ]
then
export NIGHTLY_BUILD=TRUE
fi

export MSCORE_RELEASE_CHANNEL=$(grep '^[[:blank:]]*set *( *MSCORE_RELEASE_CHANNEL' CMakeLists.txt | awk -F \" '{print $2}')
