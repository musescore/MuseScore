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

# This is module for convert mscz files to various formats (image, pdf, audio and etc).
# Main goal - determine what need to do and call necessary functions.
# Usually use from command line.

declare_module(converter)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/convertermodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/convertermodule.h
    ${CMAKE_CURRENT_LIST_DIR}/convertercodes.h
    ${CMAKE_CURRENT_LIST_DIR}/iconvertercontroller.h

    ${CMAKE_CURRENT_LIST_DIR}/api/converterapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/converterapi.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/convertercontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/convertercontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/converterutils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/converterutils.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/compat/backendapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/compat/backendapi.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/compat/backendjsonwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/compat/backendjsonwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/compat/notationmeta.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/compat/notationmeta.h
    )

setup_module()

