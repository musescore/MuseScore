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

if [[ "$NIGHTLY_BUILD" = "TRUE" ]]
then
cp -f build/travis/resources/splash-nightly.png  mscore/data/splash.png
cp -f build/travis/resources/mscore-nightly.icns mscore/data/mscore.icns
#else
#python build/add-mc-keys.py $MC_CONSUMER_KEY $MC_CONSUMER_SECRET
fi

make -f Makefile.osx ci BUILD_NUMBER=${TRAVIS_BUILD_NUMBER}


mkdir -p applebuild/mscore.app/Contents/Resources/Frameworks

# install lame
wget -c --no-check-certificate -nv -O musescore_dependencies_macos.zip  http://utils.musescore.org.s3.amazonaws.com/musescore_dependencies_macos.zip
unzip musescore_dependencies_macos.zip -d applebuild/mscore.app/Contents/Resources/Frameworks

#install Sparkle
mkdir -p applebuild/mscore.app/Contents/Frameworks
cp -Rf ~/Library/Frameworks/Sparkle.framework applebuild/mscore.app/Contents/Frameworks

if [[ "$NIGHTLY_BUILD" = "TRUE" ]]
then # Build is marked UNSTABLE inside CMakeLists.txt
build/package_mac $BRANCH-$REVISION
PACKAGE_NAME=MusescoreNightly_10.10
DMGFILE=applebuild/$PACKAGE_NAME-$DATE-$BRANCH-$REVISION.dmg
DMGFILENAME=$PACKAGE_NAME-$DATE-$BRANCH-$REVISION.dmg
mv applebuild/$PACKAGE_NAME-$BRANCH-$REVISION.dmg $DMGFILE
else
build/package_mac
PACKAGE_NAME=Musescore
DMGFILE=applebuild/$PACKAGE_NAME-*.dmg
fi

# send nightly update to S3
VERSION_MAJOR=$(grep 'SET(MUSESCORE_VERSION_MAJOR' CMakeLists.txt | cut -d \" -f2)
VERSION_MINOR=$(grep 'SET(MUSESCORE_VERSION_MINOR' CMakeLists.txt | cut -d \" -f2)
VERSION_PATCH=$(grep 'SET(MUSESCORE_VERSION_PATCH' CMakeLists.txt | cut -d \" -f2)
BUILD_NUMBER=${TRAVIS_BUILD_NUMBER}
MUSESCORE_VERSION=${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${BUILD_NUMBER}
SHORT_DATE="$(date -u +%Y-%m-%d)"
#date -R is not supporte !?
RSS_DATE="$(LANG=C date +'%a, %d %b %Y %H:%M:%S %z')"
FILESIZE="$(wc -c $DMGFILE | awk '{print $1}')"
APPCAST_URL=$(defaults read `pwd`/applebuild/mscore.app/Contents/Info.plist SUFeedURL)
GIT_LOG=$(./build/travis/job_macos/generateGitLog.sh)

#install artifacts
curl -sL https://raw.githubusercontent.com/travis-ci/artifacts/master/install | bash

echo "<rss xmlns:sparkle=\"http://www.andymatuschak.org/xml-namespaces/sparkle\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" version=\"2.0\">
<channel>
<title>Musescore development channel</title>
<link>
${APPCAST_URL}
</link>
<description>Most recent changes with links to updates.</description>
<language>en</language>
<item>
<title>Musescore ${MUSESCORE_VERSION} ${REVISION}</title>
<description>
<![CDATA[
${GIT_LOG}
]]>
</description>
<pubDate>${RSS_DATE}</pubDate>
<enclosure url=\"https://ftp.osuosl.org/pub/musescore-nightlies/macosx/${DMGFILENAME}\" sparkle:version=\"${MUSESCORE_VERSION}\" length=\"${FILESIZE}\" type=\"application/octet-stream\"/>
</item>
</channel>
</rss>" >> appcast.xml

#invalidate both Win and Mac appcast.xml files
export ARTIFACTS_KEY=$UPDATE_S3_KEY
export ARTIFACTS_SECRET=$UPDATE_S3_SECRET
export ARTIFACTS_REGION=us-east-1
export ARTIFACTS_BUCKET=sparkle.musescore.org
export ARTIFACTS_CACHE_CONTROL='public, max-age=315360000'
export ARTIFACTS_PERMISSIONS=public-read
export ARTIFACTS_TARGET_PATHS="/${MSCORE_RELEASE_CHANNEL}/3/macos_10.10"
export ARTIFACTS_PATHS=appcast.xml
artifacts upload

pip install awscli
export AWS_ACCESS_KEY_ID=$UPDATE_S3_KEY
export AWS_SECRET_ACCESS_KEY=$UPDATE_S3_SECRET
export ARTIFACTS_TARGET_PATHS="/${MSCORE_RELEASE_CHANNEL}/3"
aws configure set preview.cloudfront true
aws cloudfront create-invalidation --distribution-id E3VZY4YYZZG82P --paths "${ARTIFACTS_TARGET_PATHS}/*"
