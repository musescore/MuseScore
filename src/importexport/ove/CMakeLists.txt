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

declare_module(iex_ove)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/ovemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ovemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/ioveconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/oveconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/oveconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/overeader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/overeader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importove.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/ove.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/ove.h
    )

set(MODULE_LINK
    engraving
    )

setup_module()

