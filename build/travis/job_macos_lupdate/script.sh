#!/bin/bash

set -e

# do not build mac for PR
if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
  exit 0
fi

# Translation routines
# update translation on transifex
./build/gen-qt-projectfile . > mscore.pro
lupdate mscore.pro
./build/gen-instruments-projectfile ./share/instruments > instruments.pro
lupdate instruments.pro
./build/gen-tours-projectfile ./share/tours > tours.pro
lupdate tours.pro

rm mscore.pro
rm instruments.pro
rm tours.pro

# sudo pip install transifex-client

# cat > ~/.transifexrc <<EOL
# [https://www.transifex.com]
# hostname = https://www.transifex.com
# password = $TRANSIFEX_PASSWORD
# token =
# username = $TRANSIFEX_USER
# EOL

# cp share/locale/mscore_en_US.ts share/locale/mscore_en.ts
# tx push -s
