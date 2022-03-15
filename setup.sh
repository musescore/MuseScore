#!/usr/bin/env bash

apt_packages_basic=(
  # Alphabetical order please!
  file
  git
  pkg-config
  software-properties-common # installs `add-apt-repository`
  unzip
  p7zip-full
  )

# These are the same as on Travis CI
apt_packages_standard=(
  # Alphabetical order please!
  curl
  libasound2-dev 
  libfontconfig1-dev
  libfreetype6-dev
  libfreetype6
  libgl1-mesa-dev
  libjack-dev
  libmp3lame-dev
  libnss3-dev
  libportmidi-dev
  libpulse-dev
  libsndfile1-dev
  make
  portaudio19-dev
  wget
  )

# MuseScore compiles without these but won't run without them
apt_packages_runtime=(
  # Alphabetical order please!
  libcups2
  libdbus-1-3
  libegl1-mesa-dev
  libodbc1
  libpq-dev
  libssl1.0.0
  libxcomposite-dev
  libxcursor-dev
  libxi-dev
  libxkbcommon-x11-0
  libxrandr2
  libxtst-dev
  libdrm-dev
  )

apt-get update # no package lists in Docker image
apt-get install -y --no-install-recommends \
  "${apt_packages_basic[@]}" \
  "${apt_packages_standard[@]}" \
  "${apt_packages_runtime[@]}"