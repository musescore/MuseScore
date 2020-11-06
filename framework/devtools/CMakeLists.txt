#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2020 MuseScore BVBA and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

set(MODULE devtools)

include(${PROJECT_SOURCE_DIR}/thirdparty/haw_logger/logger/logger.cmake)

set(MODULE_DEF
    -DHAW_LOGGER_QT_SUPPORT
    -DD__STDC_FORMAT_MACROS
)

set(MODULE_INCLUDE
    ${PROJECT_SOURCE_DIR}/thirdparty/google-breakpad
)

set(MODULE_SRC
    ${HAW_LOGGER_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/devtoolsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/devtoolsmodule.h

    ${CMAKE_CURRENT_LIST_DIR}/logger/log_base.h
    ${CMAKE_CURRENT_LIST_DIR}/logger/logstream.h

    ${CMAKE_CURRENT_LIST_DIR}/dump/crashhandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dump/crashhandler.h
)

set(FS_LIB )
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(FS_LIB stdc++fs)
endif()

set(MODULE_LINK
    ${FS_LIB}
    gbreakpad
)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
