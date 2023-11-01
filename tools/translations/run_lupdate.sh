#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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

cd "${BASH_SOURCE%/*}/../.." # go to repository root

if [[ "$@" = *"-no-obsolete"* ]]; then
    echo "Note: cleaning up obsolete strings"
else
    echo "Note: preserving obsolete strings (use -no-obsolete to clean them up)"
fi

LUPDATE=lupdate
SRC_DIR=src
TS_FILE=share/locale/musescore_en.ts
ARGS=(
    -recursive
    -tr-function-alias translate+=trc
    -tr-function-alias translate+=mtrc
    -tr-function-alias translate+=qtrc
    -tr-function-alias translate+=TranslatableString
    -tr-function-alias qsTranslate+=qsTrc
    -extensions cpp,h,mm,ui,qml,js
    "$@"
)

# We only need to update one ts file per "resource", that will be sent to Transifex.
# We get .ts files for other languages from Transifex.

# musescore
echo "MuseScore:"
"${LUPDATE}" "${ARGS[@]}" "${SRC_DIR}" -ts "${TS_FILE}"

echo ""

# instruments (and templates, and score orders, currently)
FAKE_HEADER_FILE=share/instruments/instrumentsxml.h
TS_FILE=share/locale/instruments_en.ts
ARGS=("$@")

echo "Instruments:"
"${LUPDATE}" "${ARGS[@]}" "${FAKE_HEADER_FILE}" -ts "${TS_FILE}"
