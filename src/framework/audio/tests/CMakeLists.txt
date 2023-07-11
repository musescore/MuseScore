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

set(MODULE_TEST audio_test)
if(OS_IS_FBSD)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /usr/local/lib/libasound.so")
endif()
set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/mocks/audioconfigurationmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/knownaudiopluginsregistermock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/audiopluginsscannerregistermock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/audiopluginsscannermock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/audiopluginmetareaderregistermock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/audiopluginmetareadermock.h

    ${CMAKE_CURRENT_LIST_DIR}/knownaudiopluginsregistertest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/registeraudiopluginsscenariotest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audioutilstest.cpp
)

set(MODULE_TEST_LINK audio)

include(${PROJECT_SOURCE_DIR}/src/framework/testing/gtest.cmake)

