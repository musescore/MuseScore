#!/bin/sh
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
set -e
LC_ALL=C
LANGUAGE=C
unset LANGUAGE
export LC_ALL
cd "$(dirname "$0")"

jq -f smufl2sym-cmd-debug.jq \
    -r <../../fonts/smufl/glyphnames.json \
    >smufl2sym-out-debug

{
	echo '    // SMuFL standard symbol IDs {{{'
	jq -f smufl2sym-cmd-SymId.jq \
	    -r <../../fonts/smufl/glyphnames.json
	echo '    // SMuFL standard symbol IDs }}}'
} >smufl2sym-out-symid.h-SymId

{
	echo '    // SMuFL standard symbol names {{{'
	jq -f smufl2sym-cmd-symNames.jq \
	    -r <../../fonts/smufl/glyphnames.json
	echo '    // SMuFL standard symbol names }}}'
} >smufl2sym-out-symnames.cpp-symNames

{
	echo '    // SMuFL standard symbol user names {{{'
	jq -f smufl2sym-cmd-symUserNames.jq \
	    --slurpfile tr smufl2sym-in-trans.json \
	    -r <../../fonts/smufl/glyphnames.json
	echo '    // SMuFL standard symbol user names }}}'
} >smufl2sym-out-symnames.cpp-symUserNames

echo 'All done!'
