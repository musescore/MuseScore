# $1 - artifact name
# $2 - build time
# $3 - artifact ftp path
# $4 - MuseScore version
# $5 - Build number
export MSCORE_RELEASE_CHANNEL=$(grep '^[[:blank:]]*set *( *MSCORE_RELEASE_CHANNEL' CMakeLists.txt | awk -F \" '{print $2}')
RSS_DATE="$2"
FILESIZE="$(wc -c $1 | awk '{print $1}')"
APPCAST_URL="https://sparkle.musescore.org/$MSCORE_RELEASE_CHANNEL/3/macos/appcast.xml"
GIT_LOG=$(C:/MuseScore/build/travis/job_macos/generateGitLog.sh)

#use dummy values for now
echo "<rss xmlns:sparkle=\"http://www.andymatuschak.org/xml-namespaces/sparkle\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" version=\"2.0\">
<channel>
<title>MuseScore development channel</title>
<link>
${APPCAST_URL}
</link>
<description>Most recent changes with links to updates.</description>
<language>en</language>
<item>
<title>MuseScore $4 $5</title>
<description>
<![CDATA[
${GIT_LOG}
]]>
</description>
<pubDate>${RSS_DATE}</pubDate>
<enclosure url=\"$3\" sparkle:version=\"$4\" length=\"${FILESIZE}\" type=\"application/octet-stream\"/>
</item>
</channel>
</rss>" >> appcast.xml

#install artifacts
curl -sL https://raw.githubusercontent.com/travis-ci/artifacts/master/install | bash

export ARTIFACTS_KEY=k68f3wMKIC5AzrfNMuC4kdPaxzvKdFVkRsietUKqc+E=
export ARTIFACTS_SECRET=IbpdpiHzGfMasaSA6uGrskE4xu9wE+HzElW7tIDOUww+ivHj+gN+mPgUHKCcV9Cn
export ARTIFACTS_REGION=us-east-1
export ARTIFACTS_BUCKET=sparkle.musescore.org
export ARTIFACTS_CACHE_CONTROL='public, max-age=315360000'
export ARTIFACTS_PERMISSIONS=public-read
export ARTIFACTS_TARGET_PATHS="/${MSCORE_RELEASE_CHANNEL}/3/macos/"
export ARTIFACTS_PATHS=appcast.xml
artifacts upload
