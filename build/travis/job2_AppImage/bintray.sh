#!/bin/bash

# Push AppImages and related metadata to Bintray (https://bintray.com/docs/api/)
# Adapted from: https://github.com/probonopd/AppImages/blob/master/bintray.sh

# The MIT License (MIT)
#
# Copyright (c) 2004-15 Simon Peter
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

trap 'exit 1' ERR

which curl || exit 1
which bsdtar || exit 1 # https://github.com/libarchive/libarchive/wiki/ManPageBsdtar1 ; isoinfo cannot read zisofs
which grep || exit 1
which zsyncmake || exit 1

# Do not upload artefacts generated as part of a pull request
if [ $(env | grep TRAVIS_PULL_REQUEST ) ] ; then
  if [ "$TRAVIS_PULL_REQUEST" != "false" ] ; then
    echo "Not uploading AppImage since this is a pull request."
    exit 0
  fi
fi

BINTRAY_USER=$BINTRAY_USER # env
BINTRAY_API_KEY=$BINTRAY_API_KEY # env

BINTRAY_REPO_OWNER="musescore"
WEBSITE_URL="http://musescore.org"
ISSUE_TRACKER_URL="https://musescore.org/project/issues"
VCS_URL="https://github.com/musescore/MuseScore.git" # Mandatory for packages in free Bintray repos
LICENSE="GPL-2.0"

API="https://api.bintray.com"
FILE="$1"

[ -f "$FILE" ] || { echo "Please provide a valid path to a file" >&2 ; exit 1 ;}

# GENERAL NAMING SCHEME FOR APPIMAGES:
# File: <appName>-<version>-<arch>.AppImage
#
# MUSESCORE NAMING SCHEME:
# File:    MuseScore-X.Y.Z-<arch>.AppImage (e.g. MuseScore-2.0.3-x86_64)
# Version: X.Y.Z
# Package: MuseScore-Linux
#
# NIGHTLY NAMING SCHEME:
# File:    MuseScoreNightly-<datetime>-<branch>-<commit>-<arch>.AppImage
#    (e.g. MuseScoreNightly-201601151332-master-f53w6dg-x86_64.AppImage)
# Version: <datetime>-<branch>-<commit> (e.g. 201601151332-master-f53w6dg)
# Package: MuseScoreNightly-<branch> (e.g. MuseScoreNightly-master)

# Read app name from file name (get characters before first dash)
APPNAME="$(basename "$FILE" | sed -r 's|^([^-]*)-.*$|\1|')"

# Read version from the file name (get characters between first and last dash)
VERSION="$(basename "$FILE" | sed -r 's|^[^-]*-(.*)-[^-]*$|\1|')"

# Read architecture from file name (characters between last dash and .AppImage)
ARCH="$(basename "$FILE" | sed -r 's|^.*-([^-]*)\.AppImage$|\1|')"

case "${ARCH}" in
  x86_64|amd64 )
    SYSTEM="64 bit (${ARCH})"
    ;;
  i686|i386 )
    SYSTEM="32 bit (${ARCH})"
    ;;
  *[Aa][Rr][Mm]* )
    SYSTEM="ARM (${ARCH})"
    ;;
  * )
    echo "Error: unrecognised architecture '${ARCH}'"
    exit 1
    ;;
esac

FILE_UPLOAD_PATH="$ARCH/$(basename "${FILE}")"

if [ "${APPNAME}" == "MuseScore" ]; then
  # Upload a new version but don't publish it (invisible until published)
  url_query="" # Don't publish, don't overwrite existing files with same name
  PCK_NAME="$APPNAME-Linux"
  BINTRAY_REPO="MuseScore"
  LABELS="[\"music\", \"audio\", \"MIDI\", \"AppImage\"]"
elif  [ "${APPNAME}" == "MuseScoreNightly" ]; then
  # Upload and publish a new nightly build (visible to users immediately)
  url_query="publish=1&override=1" # Automatically publish, overwrite exiting

  # Get Git branch from $VERSION (get characters between first and last dash)
  BRANCH="$(echo $VERSION | sed -r 's|^[^-]*-(.*)-[^-]*$|\1|')"

  PCK_NAME="$APPNAME-$BRANCH"
  BINTRAY_REPO="nightlies-linux"
  LABELS="[\"nightly\", \"unstable\", \"testing\"]"
else
  echo "Error: AppName not recognised." >&2
  exit 1
fi

if [ ! $(env | grep BINTRAY_USER ) ] ; then
  echo "Environment variable \$BINTRAY_USER missing"
  exit 1
fi

if [ ! $(env | grep BINTRAY_API_KEY ) ] ; then
  echo "Environment variable \$BINTRAY_API_KEY missing"
  exit 1
fi

CURL="curl -u${BINTRAY_USER}:${BINTRAY_API_KEY} -H Content-Type:application/json -H Accept:application/json"

#exit 0

# Get metadata from the desktop file inside the AppImage
DESKTOP=$(bsdtar -tf "${FILE}" | grep ^./[^/]*.desktop$ | head -n 1)
# Extract the description from the desktop file

echo "* DESKTOP $DESKTOP"

#PCK_NAME=$(bsdtar -f "${FILE}" -O -x ./"${DESKTOP}" | grep -e "^Name=" | head -n 1 | sed s/Name=//g | cut -d " " -f 1 | xargs)
#if [ "$PCK_NAME" == "" ] ; then
#  bsdtar -f "${FILE}" -O -x ./"${DESKTOP}"
#  echo "PCK_NAME missing in ${DESKTOP}, exiting"
#  exit 1
#else
#  echo "* PCK_NAME $PCK_NAME"
#fi

if [ "${APPNAME}" == "MuseScore" ]; then
  # Get description from desktop file
  DESCRIPTION=$(bsdtar -f "${FILE}" -O -x ./"${DESKTOP}" | grep -e "^Comment=" | sed s/Comment=//g)
elif [ "${APPNAME}" == "MuseScoreNightly" ]; then
  # Use custom description for nightly builds
  DESCRIPTION="Automated builds of the $BRANCH development branch for Linux systems. FOR TESTING PURPOSES ONLY!"
fi

ICONNAME=$(bsdtar -f "${FILE}" -O -x "${DESKTOP}" | grep -e "^Icon=" | sed s/Icon=//g)

# Look for .DirIcon first
ICONFILE=$(bsdtar -tf "${FILE}" | grep /.DirIcon$ | head -n 1 )

# Look for svg next
if [ "$ICONFILE" == "" ] ; then
 ICONFILE=$(bsdtar -tf "${FILE}" | grep ${ICONNAME}.svg$ | head -n 1 )
fi

# If there is no svg, then look for pngs in usr/share/icons and pick the largest
if [ "$ICONFILE" == "" ] ; then
  ICONFILE=$(bsdtar -tf "${FILE}" | grep usr/share/icons.*${ICONNAME}.png$ | sort -V | tail -n 1 )
fi

# If there is still no icon, then take any png
if [ "$ICONFILE" == "" ] ; then
  ICONFILE=$(bsdtar -tf "${FILE}" | grep ${ICONNAME}.png$ | head -n 1 )
fi

if [ ! "$ICONFILE" == "" ] ; then
  echo "* ICONFILE $ICONFILE"
  bsdtar -f "${FILE}" -O -x "${ICONFILE}" > /tmp/_tmp_icon
  echo "xdg-open /tmp/_tmp_icon"
fi

# Check if there is appstream data and use it
APPDATANAME=$(echo ${DESKTOP} | sed 's/.desktop/.appdata.xml/g' | sed 's|./||'  )
APPDATAFILE=$(bsdtar -tf "${FILE}" | grep ${APPDATANAME}$ | head -n 1 || true)
APPDATA=$(bsdtar -f "${FILE}" -O -x "${APPDATAFILE}" || true)
if [ "$APPDATA" == "" ] ; then
  echo "* APPDATA missing"
else
  echo "* APPDATA found"
  DESCRIPTION=$(echo $APPDATA | grep -o -e "<description.*description>" | sed -e 's/<[^>]*>//g')
  WEBSITE_URL=$(echo $APPDATA | grep "homepage" | head -n 1 | cut -d ">" -f 2 | cut -d "<" -f 1)
fi

if [ "$DESCRIPTION" == "" ] ; then
  bsdtar -f "${FILE}" -O -x ./"${DESKTOP}"
  echo "DESCRIPTION missing and no Comment= in ${DESKTOP}, exiting"
  exit 1
else
  echo "* DESCRIPTION $DESCRIPTION"
fi

if [ "$VERSION" == "" ] ; then
  echo "* VERSION missing, exiting"
  exit 1
else
  echo "* VERSION $VERSION"
fi

# exit 0
##########

echo ""
echo "Creating package ${PCK_NAME}..."
    data="{
    \"name\": \"${PCK_NAME}\",
    \"desc\": \"${DESCRIPTION}\",
    \"desc_url\": \"auto\",
    \"website_url\": [\"${WEBSITE_URL}\"],
    \"vcs_url\": [\"${VCS_URL}\"],
    \"issue_tracker_url\": [\"${ISSUE_TRACKER_URL}\"],
    \"licenses\": [\"${LICENSE}\"],
    \"labels\": ${LABELS}
    }"
${CURL} -X POST -d "${data}" ${API}/packages/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}

if [ $(which zsyncmake) ] ; then
  echo ""
  echo "Embedding update information into ${FILE}..."
  # Clear ISO 9660 Volume Descriptor #1 field "Application Used"
  # (contents not defined by ISO 9660) and write URL there
  dd if=/dev/zero of="${FILE}" bs=1 seek=33651 count=512 conv=notrunc
  # Example for next line: Subsurface-_latestVersion-x86_64.AppImage
  NAMELATESTVERSION=$(echo "${FILE_UPLOAD_PATH}" | sed -e "s|${VERSION}|_latestVersion|g")
  # Example for next line: bintray-zsync|probono|AppImages|Subsurface|Subsurface-_latestVersion-x86_64.AppImage.zsync
  LINE="bintray-zsync|${BINTRAY_REPO_OWNER}|${BINTRAY_REPO}|${PCK_NAME}|${NAMELATESTVERSION}.zsync"
  echo "${LINE}" | dd of="${FILE}" bs=1 seek=33651 count=512 conv=notrunc
  echo ""
  echo "Uploading zsync file for ${FILE}..."
  # Workaround for:
  # https://github.com/probonopd/zsync-curl/issues/1
  zsyncmake -u http://dl.bintray.com/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/"${FILE_UPLOAD_PATH}" ${FILE} -o ${FILE}.zsync
  ${CURL} -T ${FILE}.zsync "${API}/content/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${PCK_NAME}/${VERSION}/"${FILE_UPLOAD_PATH}".zsync?${url_query}"
else
  echo "zsyncmake not found, skipping zsync file generation and upload"
fi

echo ""
echo "Uploading ${FILE}..."
${CURL} -T ${FILE} "${API}/content/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${PCK_NAME}/${VERSION}/"${FILE_UPLOAD_PATH}"?${url_query}"

if [ "${APPNAME}" == "MuseScoreNightly" ] && [ $(env | grep TRAVIS_JOB_ID ) ] ; then
  echo ""
  echo "Adding Travis CI log to release notes..."
  BUILD_LOG="https://api.travis-ci.org/jobs/${TRAVIS_JOB_ID}/log.txt?deansi=true"
      data='{
    "bintray": {
      "syntax": "markdown",
      "content": "'${BUILD_LOG}'"
    }
  }'
  ${CURL} -X POST -d "${data}" ${API}/packages/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${PCK_NAME}/versions/${VERSION}/release_notes
fi

# Seemingly this works only after the second time running this script - thus disabling for now (FIXME)
# echo ""
# echo "Adding ${FILE} to download list..."
# sleep 5 # Seemingly needed
#     data="{
#     \"list_in_downloads\": true
#     }"
# ${CURL} -X PUT -d "${data}" ${API}/file_metadata/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/"${FILE_UPLOAD_PATH}"
# echo "TODO: Remove earlier versions of the same architecture from the download list"

# echo ""
# echo "TODO: Uploading screenshot for ${FILE}..."
