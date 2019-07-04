#!/bin/bash

# do not build mac for PR
if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
  exit 0
fi

wget -nv https://s3.amazonaws.com/utils.musescore.org/qt5124_mac.zip
mkdir -p $QT_MACOS
unzip -qq qt5124_mac.zip -d $QT_MACOS
rm qt5124_mac.zip
