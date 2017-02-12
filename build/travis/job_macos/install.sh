#!/bin/bash

# do not build mac for PR
if [ "${TRAVIS_PULL_REQUEST}" = "true" ]; then
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
#brew install libogg libvorbis flac libsndfile portaudio

function installBottleManually {
  brew unlink $1
  rm -rf /usr/local/Cellar/$1
  tar xzvf bottles/$1*.tar.gz -C /usr/local/Cellar
  find /usr/local/Cellar/$1 -type f -name '*.pc' -exec sed -i '' 's:@@HOMEBREW_CELLAR@@:/usr/local/Cellar:g' {} +
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
which -s qmake
QT_INSTALLED=$?
QMAKE_VERSION=
if [[ $QT_INSTALLED == 0 ]]; then
  QMAKE_VERSION=$(qmake -query QT_VERSION)
fi

echo "QMAKE_VERSION $QMAKE_VERSION"
echo "QT_INSTALLED $QT_INSTALLED"
echo "QT_LONG_VERSION QT_LONG_VERSION"

if [[ "$QMAKE_VERSION" != "${QT_LONG_VERSION}" ]]; then
  rm -rf $QT_PATH
  echo "Downloading Qt"
  wget -c --no-check-certificate -nv https://download.qt.io/archive/qt/${QT_SHORT_VERSION}/${QT_LONG_VERSION}/${QT_INSTALLER_FILENAME}
  hdiutil mount ${QT_INSTALLER_FILENAME}
  cp -rf /Volumes/${QT_INSTALLER_ROOT}/${QT_INSTALLER_ROOT}.app $HOME/${QT_INSTALLER_ROOT}.app
  QT_INSTALLER_EXE=$HOME/${QT_INSTALLER_ROOT}.app/Contents/MacOS/${QT_INSTALLER_ROOT}

  echo "Installing Qt"
  ./build/travis/job_macos/extract-qt-installer $QT_INSTALLER_EXE $QT_PATH
  rm -rf $HOME/${QT_INSTALLER_ROOT}.app
else
  echo "Qt ${QT_LONG_VERSION} already installed"
fi












