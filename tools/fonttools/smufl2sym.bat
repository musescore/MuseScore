::
:: // SPDX-License-Identifier: GPL-3.0-only
:: // MuseScore-CLA-applies
:: //=============================================================================
:: //  MuseScore
:: //  Music Composition & Notation
:: //
:: //  Copyright (C) 2021 MuseScore BVBA and others
:: //
:: //  This program is free software: you can redistribute it and/or modify
:: //  it under the terms of the GNU General Public License version 3 as
:: //  published by the Free Software Foundation.
:: //
:: //  This program is distributed in the hope that it will be useful,
:: //  but WITHOUT ANY WARRANTY; without even the implied warranty of
:: //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
:: //  GNU General Public License for more details.
:: //
:: //  You should have received a copy of the GNU General Public License
:: //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
:: //=============================================================================::
@echo off

REM You need JQ for Windows from: https://stedolan.github.io/jq/download/
REM
REM Rename jq-win64.exe or jq-win32.exe to jq.exe and place it either in
REM the %PATH% or in the fonttools directory in the MuseScore tree.

REM You need DOS2UNIX for Windows from: https://waterlan.home.xs4all.nl/dos2unix.html
REM
REM Extract bin\dos2unix.exe off dos2unix-*-win64.zip or dos2unix-*-win32.zip
REM and place it either in the %PATH% or in the fonttools directory.

cd /d "%~dp0"

jq -f smufl2sym-cmd-debug.jq -r <..\..\fonts\smufl\glyphnames.json >smufl2sym-out-debug

echo     // SMuFL standard symbol IDs {{{>smufl2sym-out-symid.h-SymId
jq -f smufl2sym-cmd-SymId.jq -r <..\..\fonts\smufl\glyphnames.json >>smufl2sym-out-symid.h-SymId
echo     // SMuFL standard symbol IDs }}}>>smufl2sym-out-symid.h-SymId

echo     // SMuFL standard symbol names {{{>smufl2sym-out-symnames.cpp-symNames
jq -f smufl2sym-cmd-symNames.jq -r <..\..\fonts\smufl\glyphnames.json >>smufl2sym-out-symnames.cpp-symNames
echo     // SMuFL standard symbol names }}}>>smufl2sym-out-symnames.cpp-symNames

echo     // SMuFL standard symbol user names {{{>smufl2sym-out-symnames.cpp-symUserNames
jq -f smufl2sym-cmd-symUserNames.jq --slurpfile tr smufl2sym-in-trans.json -r <..\..\fonts\smufl\glyphnames.json >>smufl2sym-out-symnames.cpp-symUserNames
echo     // SMuFL standard symbol user names }}}>>smufl2sym-out-symnames.cpp-symUserNames

dos2unix smufl2sym-out-debug smufl2sym-out-symnames.cpp-symNames smufl2sym-out-symnames.cpp-symUserNames smufl2sym-out-symid.h-SymId

echo All done!
