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
cmake_minimum_required(VERSION 3.5)

project(cpad LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-g")

set(MU_ROOT ../..)
set(CRASHPAD_PATH ${MU_ROOT}/thirdparty/google_crashpad_client)

add_subdirectory(${CRASHPAD_PATH} google_crashpad_client)

add_executable(cpad
    main.cpp
)

target_include_directories(cpad PUBLIC
    ${CRASHPAD_PATH}
    ${CRASHPAD_PATH}/third_party/mini_chromium/mini_chromium
)

target_link_libraries(cpad
    gcrashpad
)
