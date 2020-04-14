#!/bin/bash

export QT_SHORT_VERSION=5.12
export QT_PATH=$HOME/qt
export QT_MACOS=$QT_PATH/$QT_SHORT_VERSION/clang_64
export PATH=$PATH:$QT_MACOS/bin

# Set NIGHTLY_BUILD variable if MSCORE_UNSTABLE is TRUE
if [ "$(cmake -P config.cmake | grep -o 'MSCORE_UNSTABLE  *TRUE')" ]
then
export NIGHTLY_BUILD=TRUE
fi

export MSCORE_RELEASE_CHANNEL=$(cmake -P config.cmake | sed -n -e 's/^.*MSCORE_RELEASE_CHANNEL  *//p')
