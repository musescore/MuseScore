#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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
echo "Publish Docker MuseScore"

HERE="$(dirname ${BASH_SOURCE[0]})"
MU_VERSION="3.6.2.2270279339"
ACCESS_USER="igorkorsukov" # For test, should be replaced with muse-bot
ACCESS_TOKEN=$GITHUB_TOKEN

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -v|--version) MU_VERSION="$2"; shift ;;
        -t|--token) ACCESS_TOKEN="$2"; shift ;;
        -u|--user) ACCESS_USER="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$MU_VERSION" ]; then MU_VERSION=$(cat $ARTIFACTS_DIR/env/build_version.env); fi

if [ -z "$MU_VERSION" ]; then echo "Error: Version not set"; exit 1; fi
if [ -z "$ACCESS_TOKEN" ]; then echo "Error: Token not set"; exit 1; fi
if [ -z "$ACCESS_USER" ]; then echo "Error: User not set"; exit 1; fi

echo "Login Docker"
echo $ACCESS_TOKEN | docker login ghcr.io -u $ACCESS_USER --password-stdin

echo "Push Docker"
docker push ghcr.io/musescore/converter_3:${MU_VERSION}

echo "Done!!"

