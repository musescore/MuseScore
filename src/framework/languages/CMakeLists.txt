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

declare_module(muse_languages)
set(MODULE_ALIAS muse::languages)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/languagesmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/languagesmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/languagestypes.h
    ${CMAKE_CURRENT_LIST_DIR}/languageserrors.h
    ${CMAKE_CURRENT_LIST_DIR}/ilanguagesconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/ilanguagesservice.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/languagesconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/languagesconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/languagesservice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/languagesservice.h
    )

setup_module()

