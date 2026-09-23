#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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

# On macOS the linker leaves the DWARF in the object files and puts only a
# debug map into the executable, so this has to run before packaging, while
# both are still around: macdeployqt strips the binary and package.sh deletes
# every dSYM inside the bundle.
#
# The dSYM keeps matching the shipped binary afterwards, because LC_UUID
# survives strip, install_name_tool and codesign.

set -euo pipefail

APP_BIN=applebuild/mscore.app/Contents/MacOS/mscore
APP_DSYM=applebuild/mscore.dSYM

echo "Generate dSYM"
echo "APP_BIN: ${APP_BIN}"
echo "APP_DSYM: ${APP_DSYM}"

rm -rf "${APP_DSYM}"

# dsymutil warns and still exits 0 when the binary carries no debug map
LOG=$(dsymutil "${APP_BIN}" -o "${APP_DSYM}" 2>&1 | tee /dev/stderr)

if echo "${LOG}" | grep -q "no debug symbols in executable"; then
    echo "error: no debug map in ${APP_BIN}"
    echo "       the build has no debug info, or the binary is already stripped"
    exit 1
fi

du -sh "${APP_BIN}" "${APP_DSYM}"
dwarfdump --uuid "${APP_BIN}"
dwarfdump --uuid "${APP_DSYM}"
