#!/bin/bash

set -e # exit on error
set -x # echo commands

# DEPENDENCIES

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

# Node.js
NPM_PACKAGES=(
  eclint
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

# TEST - code sanity check
if ! (cd assets && eclint check); then
  echo "$0: Code doesn't match formatting rules in 'assets/.editorconfig'."
  exit 1
fi

# CONFIGURE
# We only want to build the assets, but we still have to configure MuseScore
# so that we get access to the variables set in MuseScore's CMakeLists.txt.

mkdir build.assets
cd build.assets
cmake .. -DDOWNLOAD_ASSETS=FALSE -DONLY_BUILD_ASSETS=TRUE

# BUILD

cpus="$(getconf _NPROCESSORS_ONLN)" # get number of logical CPU cores
make -j ${cpus} assets_archive # make is faster than Xcode for assets

cd assets # because we configured MuseScore, not just the assets

assets_archive="$(ls MuseScore-assets-*.zip)"

if [[ ! -f "${assets_archive}" ]]; then
  echo "$0: Assets archive not found where it was supposed to be." >&2
  pwd # where are we?
  find . # what's here?
  exit 1
fi

# UPLOAD

if [[ "${UPLOAD_ASSETS}" ]]; then
  if [[ "${TRAVIS_REPO_SLUG}" == "musescore/MuseScore" ]] && [[ "${TRAVIS_PULL_REQUEST}" == "false" ]]; then
    # Upload assets to MuseScore server so everyone can download them
    (
      # enter subshell to set shell options locally
      set +x # protect secrets inside subshell
      echo "TODO: write command to upload ${assets_archive} to MuseScore server"
      exit 1 # remove when implemented
    )
  else
    # Upload assets to transfer.sh so developers can download and test them
    url="$(curl --upload-file "${assets_archive}" "https://transfer.sh/${file}")"
    if [[ "${url}" ]]; then
      echo "Assets uploaded to: ${url}"
    else
      echo "Assets upload failed!"
      exit 1
    fi
  fi
fi
