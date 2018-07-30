#!/bin/bash

set -e

# do not build mac for PR
if [ "${TRAVIS_PULL_REQUEST}" = "true" ]; then
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

make -f Makefile.osx ci

# install lame
wget -c --no-check-certificate -nv -O musescore_dependencies_macos.zip  http://utils.musescore.org.s3.amazonaws.com/musescore_dependencies_macos.zip
mkdir -p applebuild/mscore.app/Contents/Resources/Frameworks
unzip musescore_dependencies_macos.zip -d applebuild/mscore.app/Contents/Resources/Frameworks

if [ "$(grep '^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)' CMakeLists.txt)" ]
then # Build is marked UNSTABLE inside CMakeLists.txt
build/package_mac $BRANCH-$REVISION
PACKAGE_NAME=MuseScoreNightly
DMGFILE=applebuild/$PACKAGE_NAME-$DATE-$BRANCH-$REVISION.dmg
mv applebuild/$PACKAGE_NAME-$BRANCH-$REVISION.dmg $DMGFILE
else
build/package_mac
PACKAGE_NAME=MuseScore
DMGFILE=applebuild/$PACKAGE_NAME-*.dmg
fi

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


# update translation on transifex
make -f Makefile.osx lupdate

#sudo pip install transifex-client

cat > ~/.transifexrc <<EOL
[https://www.transifex.com]
hostname = https://www.transifex.com
password = $TRANSIFEX_PASSWORD
token =
username = $TRANSIFEX_USER
EOL

#tx push -s




