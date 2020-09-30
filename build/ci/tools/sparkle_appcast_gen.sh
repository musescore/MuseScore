#!/usr/bin/env bash

ARTIFACTS_DIR="build.artifacts"
MAJOR_VERSION=3

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -p|--platform) PLATFORM="$2"; shift ;;
        -a|--artifact) ARTIFACT_PATH="$2"; shift ;;
        -v|--version) BUILD_VERSION="$2"; shift ;;
        -r|--revision) BUILD_REVISION="$2"; shift ;;
        -c|--channel) RELEASE_CHANNEL="$2"; shift ;;
        -u|--url) UPDATE_URL="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

# try get default environment
if [ -z "$ARTIFACT_PATH" ]; then ARTIFACT_PATH=$ARTIFACTS_DIR/$(cat $ARTIFACTS_DIR/env/artifact_name.env); fi
if [ -z "$BUILD_VERSION" ]; then BUILD_VERSION="$(cat $ARTIFACTS_DIR/env/build_version.env)"; fi
if [ -z "$BUILD_REVISION" ]; then BUILD_REVISION="$(cat $ARTIFACTS_DIR/env/build_revision.env)"; fi
if [ -z "$RELEASE_CHANNEL" ]; then RELEASE_CHANNEL="$(cat $ARTIFACTS_DIR/env/release_channel.env)"; fi

# check args
if [ -z "$PLATFORM" ]; then echo "error: not set PLATFORM"; exit 1; fi
if [ -z "$ARTIFACT_PATH" ]; then echo "error: not set ARTIFACT_PATH"; exit 1; fi
if [ -z "$BUILD_VERSION" ]; then echo "error: not set BUILD_VERSION"; exit 1; fi
if [ -z "$BUILD_REVISION" ]; then echo "error: not set BUILD_REVISION"; exit 1; fi
if [ -z "$RELEASE_CHANNEL" ]; then echo "error: not set RELEASE_CHANNEL"; exit 1; fi
if [ -z "$UPDATE_URL" ]; then echo "error: not set UPDATE_URL"; exit 1; fi

echo "MAJOR_VERSION: $MAJOR_VERSION"
echo "PLATFORM: $PLATFORM"
echo "ARTIFACT_PATH: $ARTIFACT_PATH"
echo "BUILD_VERSION: $BUILD_VERSION"
echo "BUILD_REVISION: $BUILD_REVISION"
echo "RELEASE_CHANNEL: $RELEASE_CHANNEL"
echo "UPDATE_URL: $UPDATE_URL"


RSS_DATE="$(LANG=C date +'%a, %d %b %Y %H:%M:%S %z')"
FILESIZE="$(wc -c ${ARTIFACT_PATH} | awk '{print $1}')"
APPCAST_URL="https://sparkle.musescore.org/$RELEASE_CHANNEL/$MAJOR_VERSION/$PLATFORM/appcast.xml"
GIT_LOG=$(build/ci/tools/generateGitLog.sh)

#use dummy values for now
echo "<rss xmlns:sparkle=\"http://www.andymatuschak.org/xml-namespaces/sparkle\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" version=\"2.0\">
<channel>
<title>MuseScore</title>
<link>
${APPCAST_URL}
</link>
<description>Most recent changes with links to updates.</description>
<language>en</language>
<item>
<title>MuseScore $BUILD_VERSION $BUILD_REVISION</title>
<description>
<![CDATA[
${GIT_LOG}
]]>
</description>
<pubDate>${RSS_DATE}</pubDate>
<enclosure url=\"$UPDATE_URL\" sparkle:version=\"$BUILD_VERSION\" length=\"${FILESIZE}\" type=\"application/octet-stream\"/>
</item>
</channel>
</rss>" >> $ARTIFACTS_DIR/appcast.xml
