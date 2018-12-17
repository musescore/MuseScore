#!/bin/bash

# do not build mac for PR
if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
  exit 0
fi

wget -nv http://utils.musescore.org.s3.amazonaws.com/qt5120_mac.zip
mkdir -p $QT_MACOS
unzip -qq qt5120_mac.zip -d $QT_MACOS
rm qt5120_mac.zip
