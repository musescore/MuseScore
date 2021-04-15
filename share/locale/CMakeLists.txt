#[[
// SPDX-License-Identifier: GPL-3.0-only
// MuseScore-CLA-applies
//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 3 as
//  published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//=============================================================================]]

install(DIRECTORY ./
    DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}locale
    FILES_MATCHING
    REGEX ".*\\.qm"
    REGEX ".*_en\\.qm" EXCLUDE
    REGEX "languages\\.xml")

install(DIRECTORY ${QT_INSTALL_PREFIX}/translations/
    DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}locale
    FILES_MATCHING
    REGEX "qt_.*\\.qm"
    REGEX "qt_help_.*\\.qm" EXCLUDE
    REGEX "qtbase_.*\\.qm"
    REGEX "qtmultimedia_.*\\.qm"
    REGEX "qtquick1_.*\\.qm"
    REGEX "qtscript_.*\\.qm"
    REGEX "qtxmlpatterns_.*\\.qm"
    REGEX "qt*_en\\.qm" EXCLUDE)
