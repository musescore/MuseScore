# $1 - artifact name
# $2 - artifact ftp path
# $3 - MuseScore version
# $4 - Build number

export MSCORE_RELEASE_CHANNEL=$(grep '^[[:blank:]]*set *( *MSCORE_RELEASE_CHANNEL' CMakeLists.txt | awk -F \" '{print $2}')
RSS_DATE="$(LANG=C date +'%a, %d %b %Y %H:%M:%S %z')"
FILESIZE="$(wc -c $1 | awk '{print $1}')"
APPCAST_URL="https://sparkle.musescore.org/$MSCORE_RELEASE_CHANNEL/3/win/appcast.xml"
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