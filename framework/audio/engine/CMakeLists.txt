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

set(MODULE audio_engine)

include(${CMAKE_CURRENT_LIST_DIR}/3rdparty/soloud.cmake)

include(${PROJECT_SOURCE_DIR}/build/platform.cmake)
if (PLATFORM_WINDOWS)
    message(WARNING "Not implemented audio driver for Windows")
    set(DRIVER_SRC
    )
elseif(PLATFORM_LINUX)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/platform/lin/linuxaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/platform/lin/linuxaudiodriver.h
    )
elseif(PLATFORM_OSX)
    message(WARNING "Not implemented audio driver for OSX")
    set(DRIVER_SRC
    )
endif()

set(MODULE_SRC
    ${SOLOUD_SRC}
    ${DRIVER_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/audioenginemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audioenginemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiodriver.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioengine.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiosource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioengine.h
    )

set(MODULE_DEF
    ${SOLOUD_DEF}
    )

set(MODULE_INCLUDE
    ${SOLOUD_INC}
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
