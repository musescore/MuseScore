#!/bin/bash
##
## // SPDX-License-Identifier: GPL-3.0-only
## // MuseScore-CLA-applies
## //=============================================================================
## //  MuseScore
## //  Music Composition & Notation
## //
## //  Copyright (C) 2021 MuseScore BVBA and others
## //
## //  This program is free software: you can redistribute it and/or modify
## //  it under the terms of the GNU General Public License version 3 as
## //  published by the Free Software Foundation.
## //
## //  This program is distributed in the hope that it will be useful,
## //  but WITHOUT ANY WARRANTY; without even the implied warranty of
## //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## //  GNU General Public License for more details.
## //
## //  You should have received a copy of the GNU General Public License
## //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
## //=============================================================================##

TRANSIFEX_USER=""
TRANSIFEX_PASSWORD=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -u|--user) TRANSIFEX_USER="$2"; shift ;;
        -p|--password) TRANSIFEX_PASSWORD="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$TRANSIFEX_USER" ]; then echo "error: not set TRANSIFEX_USER"; exit 1; fi
if [ -z "$TRANSIFEX_PASSWORD" ]; then echo "error: not set TRANSIFEX_PASSWORD"; exit 1; fi

apt install python3-setuptools
pip3 install transifex-client

cat > ~/.transifexrc <<EOL
[https://www.transifex.com]
hostname = https://www.transifex.com
password = $TRANSIFEX_PASSWORD
token =
username = $TRANSIFEX_USER
EOL

echo "tx version: $(tx --version)"
#tx push -s
