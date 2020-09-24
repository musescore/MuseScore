#!/usr/bin/env bash

echo "Setup MacOS build environment"

curl -LO https://github.com/macports/macports-base/releases/download/v2.6.2/MacPorts-2.6.2-10.15-Catalina.pkg
sudo installer -verbose -pkg MacPorts-2.6.2-10.15-Catalina.pkg -target /
rm MacPorts-2.6.2-10.15-Catalina.pkg
export PATH="/opt/local/bin:/opt/local/sbin:$PATH"
export MACOSX_DEPLOYMENT_TARGET=10.10
echo -e "universal_target 10.10\nmacosx_deployment_target 10.10\nmacosx_sdk_version 10.10" | sudo tee -a /opt/local/etc/macports/macports.conf
sudo port install git pkgconfig cmake
sudo port -s install libsndfile lame portaudio jack
export QT_SHORT_VERSION=5.15.1
export QT_PATH=$HOME/Qt
export QT_MACOS=$QT_PATH/$QT_SHORT_VERSION/clang_64
export PATH=$PATH:$QT_MACOS/bin
echo "::set-env name=PATH::${PATH}"
wget -nv -O qt5.zip https://s3.amazonaws.com/utils.musescore.org/Qt5151_mac.zip
mkdir -p $QT_MACOS
unzip -qq qt5.zip -d $QT_MACOS
rm qt5.zip


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