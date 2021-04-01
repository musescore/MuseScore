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

set(MODULE midi)

include(GetPlatformInfo)
if (OS_IS_WIN)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidiinport.h
    )
elseif(OS_IS_LIN)
    find_package(ALSA REQUIRED)
    set(MODULE_LINK ${ALSA_LIBRARIES} pthread )
    set(MODULE_INCLUDE ${MODULE_INCLUDE} ${ALSA_INCLUDE_DIRS} )

    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/alsamidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/alsamidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/alsamidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/alsamidiinport.h
    )
elseif(OS_IS_MAC)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidiinport.h
    )
endif()


set(MODULE_SRC
    ${DRIVER_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/midimodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/midimodule.h
    ${CMAKE_CURRENT_LIST_DIR}/imidiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/imidiinport.h
    ${CMAKE_CURRENT_LIST_DIR}/imidioutport.h
    ${CMAKE_CURRENT_LIST_DIR}/imidiportdatasender.h
    ${CMAKE_CURRENT_LIST_DIR}/midievent.h
    ${CMAKE_CURRENT_LIST_DIR}/miditypes.h
    ${CMAKE_CURRENT_LIST_DIR}/midierrors.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidioutport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidioutport.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidiinport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidiinport.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiportdatasender.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiportdatasender.h
    ${CMAKE_CURRENT_LIST_DIR}/view/mididevicemappingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/mididevicemappingsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/editmidimappingmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/editmidimappingmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/devtools/midiportdevmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/devtools/midiportdevmodel.h
    )

set(MODULE_LINK
    ${MODULE_LINK}
    qzip
    )

include(GetPlatformInfo)
if (OS_IS_WIN)
    set(MODULE_LINK ${MODULE_LINK} winmm)
endif(OS_IS_WIN)

set(MODULE_QRC midi.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
