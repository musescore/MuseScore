#!/bin/bash
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

command -v s3cmd >/dev/null 2>&1 || { echo "error: not found 's3cmd'" >&2; exit 1; }

echo "s3cmd: $(s3cmd --version)"

echo "Updating translation on s3..."
SCRIPT_PATH=$(dirname $0)
python3 $SCRIPT_PATH/tx2s3.py
echo "Translation updated"
