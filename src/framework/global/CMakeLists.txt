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

set(MODULE global)

include(${CMAKE_CURRENT_LIST_DIR}/modularity/modularity.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/async/async.cmake)
include(${PROJECT_SOURCE_DIR}/thirdparty/haw_logger/logger/logger.cmake)
include(${PROJECT_SOURCE_DIR}/thirdparty/haw_profiler/src/profiler.cmake)

set(MODULE_DEF
    -DHAW_LOGGER_QT_SUPPORT
)

set(MODULE_SRC
    ${MODULARITY_SRC}
    ${ASYNC_SRC}
    ${HAW_LOGGER_SRC}
    ${HAW_PROFILER_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/globalmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/globalmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/globaltypes.h
    ${CMAKE_CURRENT_LIST_DIR}/iapplication.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/application.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/application.h
    ${CMAKE_CURRENT_LIST_DIR}/iinteractive.h
    ${CMAKE_CURRENT_LIST_DIR}/iglobalconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/io/path.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/path.h
    ${CMAKE_CURRENT_LIST_DIR}/io/device.h
    ${CMAKE_CURRENT_LIST_DIR}/log.h
    ${CMAKE_CURRENT_LIST_DIR}/logstream.h
    ${CMAKE_CURRENT_LIST_DIR}/logremover.cpp
    ${CMAKE_CURRENT_LIST_DIR}/logremover.h
    ${CMAKE_CURRENT_LIST_DIR}/dataformatter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dataformatter.h
    ${CMAKE_CURRENT_LIST_DIR}/id.cpp
    ${CMAKE_CURRENT_LIST_DIR}/id.h
    ${CMAKE_CURRENT_LIST_DIR}/val.cpp
    ${CMAKE_CURRENT_LIST_DIR}/val.h
    ${CMAKE_CURRENT_LIST_DIR}/settings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/settings.h
    ${CMAKE_CURRENT_LIST_DIR}/version.cpp
    ${CMAKE_CURRENT_LIST_DIR}/version.h
    ${CMAKE_CURRENT_LIST_DIR}/stringutils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/stringutils.h
    ${CMAKE_CURRENT_LIST_DIR}/ptrutils.h
    ${CMAKE_CURRENT_LIST_DIR}/realfn.h
    ${CMAKE_CURRENT_LIST_DIR}/runtime.cpp
    ${CMAKE_CURRENT_LIST_DIR}/runtime.h
    ${CMAKE_CURRENT_LIST_DIR}/translation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/translation.h
    ${CMAKE_CURRENT_LIST_DIR}/timer.h
    ${CMAKE_CURRENT_LIST_DIR}/ret.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ret.h
    ${CMAKE_CURRENT_LIST_DIR}/retval.h
    ${CMAKE_CURRENT_LIST_DIR}/uri.cpp
    ${CMAKE_CURRENT_LIST_DIR}/uri.h
    ${CMAKE_CURRENT_LIST_DIR}/invoker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/invoker.h
    ${CMAKE_CURRENT_LIST_DIR}/progress.h
    ${CMAKE_CURRENT_LIST_DIR}/smuflranges.cpp
    ${CMAKE_CURRENT_LIST_DIR}/smuflranges.h
    ${CMAKE_CURRENT_LIST_DIR}/xmlreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xmlreader.h
    ${CMAKE_CURRENT_LIST_DIR}/xmlwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xmlwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils.h
    ${CMAKE_CURRENT_LIST_DIR}/defer.h
    ${CMAKE_CURRENT_LIST_DIR}/sharedhashmap.h
    ${CMAKE_CURRENT_LIST_DIR}/sharedmap.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/globalconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/globalconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/interactive.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/interactive.h
)

set(FS_LIB )
# The use of `filesystem` is disabled until
# I figure out how to use it correctly for all platforms and compilers (including MinGW)
# (igor.korsukov@gmail.com)
#if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#    set(FS_LIB stdc++fs)
#endif()

set(MODULE_LINK
    ${FS_LIB}
)

if (OS_IS_MAC)
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/macos/macosinteractivehelper.mm
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/macos/macosinteractivehelper.h
    )
    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/macos/macosinteractivehelper.mm
        PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
        SKIP_PRECOMPILE_HEADERS ON
    )
    find_library(AppKit NAMES AppKit)
    set(MODULE_LINK ${MODULE_LINK} ${AppKit})
endif()

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
