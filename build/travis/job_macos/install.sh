#!/bin/bash

# do not build mac for PR
if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
  exit 0
fi

# install dependencies
wget -c --no-check-certificate -nv -O bottles.zip https://musescore.org/sites/musescore.org/files/bottles-MuseScore-3.0.zip
unzip bottles.zip

# we don't use freetype
rm bottles/freetype*

brew update

# additional dependencies
brew install jack lame
brew upgrade cmake
#brew install libogg libvorbis flac libsndfile portaudio
cmake --version

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
installBottleManually portaudio

#update ruby
rvm uninstall 2.0.0-p648
rvm uninstall 2.0.0-p643
rvm uninstall 2.0.0
rvm get head

#install Qt
#which -s qmake
#QT_INSTALLED=$?
#QMAKE_VERSION=
#if [[ $QT_INSTALLED == 0 ]]; then
#  QMAKE_VERSION=$(qmake -query QT_VERSION)
#fi
#
#echo "QMAKE_VERSION $QMAKE_VERSION"
#echo "QT_INSTALLED $QT_INSTALLED"
#echo "QT_LONG_VERSION QT_LONG_VERSION"
#
#if [[ "$QMAKE_VERSION" != "${QT_LONG_VERSION}" ]]; then
#  rm -rf $QT_PATH
#  echo "Downloading Qt"
#  wget -c --no-check-certificate -nv https://download.qt.io/archive/qt/${QT_SHORT_VERSION}/${QT_LONG_VERSION}/${QT_INSTALLER_FILENAME}
#  hdiutil mount ${QT_INSTALLER_FILENAME}
#  cp -rf /Volumes/${QT_INSTALLER_ROOT}/${QT_INSTALLER_ROOT}.app $HOME/${QT_INSTALLER_ROOT}.app
#  QT_INSTALLER_EXE=$HOME/${QT_INSTALLER_ROOT}.app/Contents/MacOS/${QT_INSTALLER_ROOT}
#
#  echo "Installing Qt"
#  ./build/travis/job_macos/extract-qt-installer $QT_INSTALLER_EXE $QT_PATH
#  rm -rf $HOME/${QT_INSTALLER_ROOT}.app
#else
#  echo "Qt ${QT_LONG_VERSION} already installed"
#fi

wget -nv http://utils.musescore.org.s3.amazonaws.com/qt593_mac.zip
mkdir -p $QT_MACOS
unzip -qq qt593_mac.zip -d $QT_MACOS
rm qt593_mac.zip

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

#install signing certificate
if [ -n "$CERTIFICATE_OSX_PASSWORD" ]
then
    export CERTIFICATE_P12=Certificate.p12
    echo $CERTIFICATE_OSX_P12 | base64 - -D -o $CERTIFICATE_P12
    export KEYCHAIN=build.keychain
    security create-keychain -p travis $KEYCHAIN
    security default-keychain -s $KEYCHAIN
    security unlock-keychain -p travis $KEYCHAIN
    # Set keychain timeout to 1 hour for long builds
    # see http://www.egeek.me/2013/02/23/jenkins-and-xcode-user-interaction-is-not-allowed/
    security set-keychain-settings -t 3600 -l $KEYCHAIN
    security import $CERTIFICATE_P12 -k $KEYCHAIN -P "$CERTIFICATE_OSX_PASSWORD" -T /usr/bin/codesign
fi












