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
function showHelp() {
cat >&2 <<"EOF"

Update reference files in mtest based on test results in build.debug/mtest.

  Usage:    mtest/updateReferences.sh  mtest/$path

  Or:       cd  mtest  &&  ./updateReferences.sh  $path

Copies test files from build.debug/mtest/$path to mtest/$path. Test files are
all files named like *-test.* and they are renamed *-ref.* in the process.

EOF
}

path="$1"

[ "$(basename "${PWD}")" == "mtest" ] && path="mtest/${path}" && cd ..

# Some checks:

if [ "${path}" == "" ] || [ ! -d "${path}" ]; then
  showHelp
  exit 1
fi

if [ "$(ls "build.xcode/mtest/guitarpro/Debug/"*-test.*)" == "" ]; then
  echo "$0: No test files in 'build.debug/$path'. Have you run the tests?"
  exit 2
fi

# All good!

echo "Copy refs from 'build.debug/${path}' to '${path}'."

for file in build.xcode/mtest/guitarpro/*-test.*; do
  cp "$file" "${path}"/"$(basename "${file}" | sed "s|-test\.|-ref\.|")"
done
