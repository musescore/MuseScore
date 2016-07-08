#!/bin/bash

set -e

# do not build mac for PR
if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
  exit 0
fi

DATE="$(date -u +%Y%m%d%H%M)"

BRANCH="$TRAVIS_BRANCH"
[ "BRANCH" ] || BRANCH="$(git rev-parse --abbrev-ref HEAD)"

REVISION="$(echo "$TRAVIS_COMMIT" | cut -c 1-7)"
[ "REVISION" ] || REVISION="$(make -f Makefile.osx revision && cat mscore/revision.h)"

cp -f build/travis/resources/splash-nightly.png  mscore/data/splash.png
cp -f build/travis/resources/mscore-nightly.icns mscore/data/mscore.icns

make -f Makefile.osx ci
if [ "$(grep '^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)' CMakeLists.txt)" ]
then # Build is marked UNSTABLE inside CMakeLists.txt
build/package_mac $BRANCH-$REVISION
PACKAGE_NAME=MuseScoreNightly
else
build/package_mac
PACKAGE_NAME=MuseScore
fi

DMGFILE=applebuild/$PACKAGE_NAME-$DATE-$BRANCH-$REVISION.dmg

mv applebuild/$PACKAGE_NAME-$BRANCH-$REVISION.dmg $DMGFILE

scp -C -i $HOME/.ssh/osuosl_nighlies_rsa $DMGFILE musescore-nightlies@ftp-osl.osuosl.org:ftp/macosx
ssh musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/macosx; ls MuseScoreNightly* -t | tail -n +41 | xargs rm; ~/trigger-musescore-nightlies"
