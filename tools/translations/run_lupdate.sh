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

# Usage:
# - use the LUPDATE_ARGS environment variable to specify arguments for `lupdate`
# - use the POSTPROCESS_ARGS environment variable to specify arugments for the postprocessing script

set -eo pipefail

cd "${BASH_SOURCE%/*}/../.." # go to repository root

if [[ "$LUPDATE_ARGS" = *"-no-obsolete"* ]]; then
    echo "Note: cleaning up obsolete strings"
else
    echo "Note: preserving obsolete strings (set LUPDATE_ARGS to \"-no-obsolete\" to clean them up)"
fi

LUPDATE=lupdate
SRC_DIR=src
TS_FILE=share/locale/musescore_en.ts
DEFAULT_LUPDATE_ARGS=(
    -recursive
    -tr-function-alias translate+=trc
    -tr-function-alias translate+=mtrc
    -tr-function-alias translate+=qtrc
    -tr-function-alias translate+=TranslatableString
    -tr-function-alias qsTranslate+=qsTrc
    -extensions cpp,h,mm,ui,qml,js
)

run_indented() {
    "$@" > >(sed 's/^/    /') 2> >(sed 's/^/    /' >&2)
}

# We only need to update one ts file per "resource", that will be sent to Transifex.
# We get .ts files for other languages from Transifex.

# musescore
echo "MuseScore:"
echo "Running" "${LUPDATE}" "${DEFAULT_LUPDATE_ARGS[@]}" ${LUPDATE_ARGS} "${SRC_DIR}" -ts "${TS_FILE}"
run_indented "${LUPDATE}" "${DEFAULT_LUPDATE_ARGS[@]}" ${LUPDATE_ARGS} "${SRC_DIR}" -ts "${TS_FILE}"

echo ""

# instruments (and templates, and score orders, currently)
FAKE_HEADER_FILE=share/instruments/instrumentsxml.h
TS_FILE=share/locale/instruments_en.ts
DEFAULT_LUPDATE_ARGS=()

echo "Instruments:"
echo "Running" "${LUPDATE}" "${DEFAULT_LUPDATE_ARGS[@]}" ${LUPDATE_ARGS} "${FAKE_HEADER_FILE}" -ts "${TS_FILE}"
run_indented "${LUPDATE}" "${DEFAULT_LUPDATE_ARGS[@]}" ${LUPDATE_ARGS} "${FAKE_HEADER_FILE}" -ts "${TS_FILE}"

echo ""

echo "Postprocessing:"

POSTPROCESS="tools/translations/process_source_ts_files.py"

echo "Running" $POSTPROCESS_LAUNCHER "${POSTPROCESS}" ${POSTPROCESS_ARGS}
run_indented $POSTPROCESS_LAUNCHER "${POSTPROCESS}" ${POSTPROCESS_ARGS}
