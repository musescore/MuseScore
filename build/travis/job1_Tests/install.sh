#!/bin/bash
# Run this script from MuseScore's root directory

set -e # exit on error
set -x # echo commands

make revision
make debug CPUS=2 PREFIX="$HOME/software" COVERAGE=ON DOWNLOAD_ASSETS=OFF
make installdebug CPUS=2 PREFIX="$HOME/software"

cd build.debug/assets
make assets_archive
file="$(ls MuseScore-assets-*.zip)"
ls "${file}" # make sure file exists
url="$(curl --upload-file "${file}" "https://transfer.sh/${file}")" # TODO: upload to MuseScore server
if [[ "${url}" ]]; then
  echo "Assets uploaded to: ${url}"
else
  echo "Assets upload failed!"
  exit 1
fi
