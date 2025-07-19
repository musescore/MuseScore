#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

o="$(basename "$0")" # script name

((${BASH_VERSION%%.*} >= 4)) || { echo >&2 "$o: Error: Please upgrade Bash."; exit 1; }

set -euo pipefail # exit on error, error on unset variable, preserve errors in pipelines

function tag_exists()
{
    local tags="$(git tag --list -- "$@")"
    if [[ "${tags}" ]]; then
        # printf '%s\n' "${tags}"
        return 0
    fi
    return 1
}

function version_exists()
{
    local version="$1" label="${2-}" # label is optional
    local suffix="${label:+-${label}}" # prepend hyphen if label not empty
    version="${version%.0}" # strip patch number if it's zero
    version="${version%.0}" # strip minor number if it's zero
    tag_exists "v${version}${suffix}" "v${version}.0${suffix}" "v${version}.0.0${suffix}"
}

if [[ "${CREATE_TAG}" == 'true' ]]; then
    types="alpha|beta|rc|stable"
    if [[ ! "${RELEASE_TYPE}" =~ ^(${types})$ ]]; then
        echo >&2 "$o: Error: Creating tag with RELEASE_TYPE '${RELEASE_TYPE}'."
        echo >&2 "Allowed release types for tags are: ${types//|/, }."
        exit 1
    fi
else
    pattern="^[A-Za-z][A-Za-z0-9._]*[A-Za-z0-9]$"
    if [[ ! "${RELEASE_TYPE}" =~ ${pattern} ]]; then
        echo >&2 "$o: Error: Invalid RELEASE_TYPE '${RELEASE_TYPE}'."
        echo >&2 "Allowed characters are ASCII letters, numbers, dot '.' and underscore '_'."
        echo >&2 "It must start with a letter and end with a letter or number."
        echo >&2 "Regex: ${pattern}"
        exit 1
    fi
fi

VERSION="$(cmake -P version.cmake | sed -n "s|^-- MUSE_APP_VERSION ||p")"
int="(0|[1-9][0-9]*)"

if [[ ! "${VERSION}" =~ ^${int}\.${int}\.${int}$ ]]; then
    echo >&2 "$o: Badly formed version from version.cmake: ${VERSION}"
    exit 1
elif version_exists "${VERSION}"; then
    if [[ "${RELEASE_TYPE}" == 'stable' ]]; then
        echo >&2 "$o: Error: Stable tag already exists for version ${VERSION}."
    else
        echo >&2 "$o: Error: Creating pre-release for v${VERSION}, which already has a stable tag."
    fi
    echo >&2 "You need to bump the version numbers in version.cmake (e.g. increase PATCH by 1)."
    exit 1
elif [[ "${RELEASE_TYPE}" =~ ^(beta|alpha)$ ]] && version_exists "${VERSION}" 'rc*'; then
    echo >&2 "$o: Error: Release type is '${RELEASE_TYPE}' but v${VERSION} already has a Release Candidate tag."
    echo >&2 "Please change the release type to 'rc' or bump the version numbers in version.cmake."
    exit 1
elif [[ "${RELEASE_TYPE}" == 'alpha' ]] && version_exists "${VERSION}" 'beta*'; then
    echo >&2 "$o: Error: Release type is '${RELEASE_TYPE}' but v${VERSION} already has a Beta tag."
    echo >&2 "Please change the release type to 'beta' or 'rc', or bump the version numbers in version.cmake."
    exit 1
fi

MAJOR="${VERSION%%.*}"
MINOR="${VERSION#*.}"
MINOR="${MINOR%%.*}"
PATCH="${VERSION#*.*.}"

if ((PATCH > 0)); then
    older_version="${MAJOR}.${MINOR}.$((PATCH - 1))"
elif ((MINOR > 0)); then
    older_version="${MAJOR}.$((MINOR - 1)).0"
elif ((MAJOR > 0)); then
    older_version="$((MAJOR - 1)).0.0"
else
    echo >&2 "$o: Error: Can't release version '0.0.0'. Please check version.cmake."
    exit 1
fi

if ! version_exists "${older_version}"; then
    # Assume VERSION isn't the very first release.
    echo >&2 "$o: Error: Version is ${VERSION} but no tag exists for ${older_version}."
    echo >&2 "You need to reduce the version numbers in version.cmake to avoid skipping releases."
    exit 1
fi

RELEASE_TYPE_COUNT=1

if [[ "${RELEASE_TYPE}" == 'stable' ]]; then
    VERSION_LABEL=""
else
    label="${RELEASE_TYPE}"
    if version_exists "${VERSION}" "${label}" || version_exists "${VERSION}" "${label}.1"; then
        while true; do
            label="${RELEASE_TYPE}.$((++RELEASE_TYPE_COUNT))"
            version_exists "${VERSION}" "${label}" || break
        done
    fi
    VERSION_LABEL="${label}"
fi

PRETTY_VERSION="${MAJOR}.${MINOR}.${PATCH}"     # e.g. '4.0.0'
TAG_NAME="v${PRETTY_VERSION}"                   # e.g. 'v4.0.0'

if [[ "${VERSION_LABEL}" ]]; then
    TAG_NAME="${TAG_NAME}-${VERSION_LABEL}"     # e.g. 'v4.0.0-rc.2'
fi

if tag_exists "${TAG_NAME}"; then
    echo >&2 "$o: Error: '${TAG_NAME}' tag already exists."
    echo >&2 "This shouldn't happen. Check for bugs in the script code."
    exit 1
fi

case "${RELEASE_TYPE}" in
stable) PRETTY_RELEASE_TYPE="Release";;
rc)     PRETTY_RELEASE_TYPE="Release Candidate";;
beta)   PRETTY_RELEASE_TYPE="Beta";;
alpha)  PRETTY_RELEASE_TYPE="Alpha";;
*)      PRETTY_RELEASE_TYPE="${RELEASE_TYPE}";;
esac

RELEASE_LABEL="${PRETTY_RELEASE_TYPE}"

if ((RELEASE_TYPE_COUNT > 1)); then
    RELEASE_LABEL="${RELEASE_LABEL} ${RELEASE_TYPE_COUNT}"
fi

echo "TAG_NAME=${TAG_NAME}"
echo "RELEASE_NAME=MuseScore Studio ${PRETTY_VERSION} ${RELEASE_LABEL}"
