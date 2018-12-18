#!/bin/bash

set -e # exit on error
set -x # echo commands

# upgrade older packages already installed on Travis CI
HOMEBREW_UPGRADES=(
  # cmake # can't upgrade without updating homebrew itself
  # node # npm - Travis version seems recent enough
  )

# new packages not already installed on Travis CI
HOMEBREW_FORMULAE=(
  imagemagick
  pngcrush
  )

# GUI applications
HOMEBREW_CASKS=(
  inkscape
  )

NPM_PACKAGES=(
  google-font-installer
  svgo
  )

GOOGLE_FONTS=(
  Raleway
  Roboto
  )

export HOMEBREW_NO_AUTO_UPDATE="1" # brew takes ages to update

# brew upgrade "${HOMEBREW_UPGRADES[@]}"
brew install "${HOMEBREW_FORMULAE[@]}"
brew cask install "${HOMEBREW_CASKS[@]}"

npm config set color false # npm's colors don't behave with Travis
npm install -g "${NPM_PACKAGES[@]}"

for font in "${GOOGLE_FONTS[@]}"; do
  gfi install "${font}" # can't install more than one at a time
done

# get number of CPUs so we can launch this many parallel build jobs
CPUS="$( getconf _NPROCESSORS_ONLN 2>/dev/null \
      || getconf NPROCESSORS_ONLN 2>/dev/null \
      || echo 1 )"

# We only want to build the assets, but we still have to configure MuseScore
# so that we get access to the variables set in MuseScore's CMakeLists.txt.
mkdir build.assets
cd build.assets
cmake .. -DDOWNLOAD_ASSETS=FALSE -DONLY_BUILD_ASSETS=TRUE
make -j ${CPUS} assets_archive # make is faster than Xcode for custom commands

cd assets # because we configured MuseScore, not just the assets

# basic CI test:
if ! git diff --no-index ../../assets/assets-manifest.txt assets-manifest.txt; then
  echo "Assets build: Generated manifest doesn't match the repository version."
fi

assets_archive="$(ls MuseScore-assets-*.zip)"
if [[ ! -f "${assets_archive}" ]]; then
  echo "Assets archive not found where it was supposed to be."
  pwd
  find .
  exit 1
fi
