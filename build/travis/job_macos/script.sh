#!/bin/bash

set -e

# do not build mac for PR
if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
  exit 0
fi

DATE="$(date -u +%Y-%m-%d-%H%M)"

BRANCH="$TRAVIS_BRANCH"
[ "BRANCH" ] || BRANCH="$(git rev-parse --abbrev-ref HEAD)"

REVISION="$(echo "$TRAVIS_COMMIT" | cut -c 1-7)"
[ "REVISION" ] || REVISION="$(make -f Makefile.osx revision && cat mscore/revision.h)"

if [ "$(grep '^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)' CMakeLists.txt)" ]
then
cp -f build/travis/resources/splash-nightly.png  mscore/data/splash.png
cp -f build/travis/resources/mscore-nightly.icns mscore/data/mscore.icns
else
python build/add-mc-keys.py $MC_CONSUMER_KEY $MC_CONSUMER_SECRET
fi

make -f Makefile.osx ci BUILD_NUMBER=${TRAVIS_BUILD_NUMBER}
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

SSH_INDENTITY=$HOME/.ssh/osuosl_nighlies_rsa

# transfer file
scp -C -i $SSH_INDENTITY $DMGFILE musescore-nightlies@ftp-osl.osuosl.org:ftp/macosx

# delete old files
ssh -i $SSH_INDENTITY musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/macosx; ls MuseScoreNightly* -t | tail -n +41 | xargs rm -f"

# create and upload index.html and RSS
python build/travis/job_macos/updateHTML.py $SSH_INDENTITY
scp -C -i $SSH_INDENTITY build/travis/job_macos/web/index.html musescore-nightlies@ftp-osl.osuosl.org:ftp/macosx
scp -C -i $SSH_INDENTITY build/travis/job_macos/web/nightly.xml musescore-nightlies@ftp-osl.osuosl.org:ftp/macosx

# trigger distribution
ssh -i $SSH_INDENTITY musescore-nightlies@ftp-osl.osuosl.org "~/trigger-musescore-nightlies"

# send nightly update to S3
VERSION_MAJOR=$(grep 'SET(MUSESCORE_VERSION_MAJOR' CMakeLists.txt | cut -d \" -f2)
VERSION_MINOR=$(grep 'SET(MUSESCORE_VERSION_MINOR' CMakeLists.txt | cut -d \" -f2)
VERSION_PATCH=$(grep 'SET(MUSESCORE_VERSION_PATCH' CMakeLists.txt | cut -d \" -f2)
BUILD_NUMBER=${TRAVIS_BUILD_NUMBER}
MUSESCORE_VERSION=${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${BUILD_NUMBER}
SHORT_DATE="$(date -u +%Y-%m-%d)"

if [ "$(grep '^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)' CMakeLists.txt)" ]
then
echo "<update>
<version>${MUSESCORE_VERSION}</version>
<revision>${REVISION}</revision>
<releaseType>nightly</releaseType>
<date>${SHORT_DATE}</date>
<description>MuseScore ${MUSESCORE_VERSION} ${REVISION}</description>
<downloadUrl>https://ftp.osuosl.org/pub/musescore-nightlies/macosx/$DMGFILE</downloadUrl>
<infoUrl>https://ftp.osuosl.org/pub/musescore-nightlies/macosx/</infoUrl>
</update>" >> update_mac_nightly.xml
export ARTIFACTS_KEY=$UPDATE_S3_KEY
export ARTIFACTS_SECRET=$UPDATE_S3_SECRET
export ARTIFACTS_REGION=us-east-1
export ARTIFACTS_BUCKET=update.musescore.org
export ARTIFACTS_CACHE_CONTROL='public, max-age=315360000'
export ARTIFACTS_PERMISSIONS=public-read
export ARTIFACTS_TARGET_PATHS="/"
export ARTIFACTS_PATHS=update_mac_nightly.xml
curl -sL https://raw.githubusercontent.com/travis-ci/artifacts/master/install | bash
artifacts upload
fi

