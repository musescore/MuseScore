# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited and others
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

declare_module(muse_audio)
set(MODULE_ALIAS muse::audio)

set(MODULE_QRC main/audio.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

include(GetPlatformInfo)

set(MODULE_LINK )
if (OS_IS_WASM)
    set(WEBENGINE_FACADE_MODE ON)
else()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/fluidsynth fluidsynth)
list(APPEND MODULE_LINK fluidsynth)

endif()

include(common/audio_common.cmake)
include(main/audio_main.cmake)
include(engine/audio_engine.cmake)
include(driver/audio_driver.cmake)

set(MODULE_SRC

    ${AUDIO_COMMON_SRC}
    ${AUDIO_MAIN_SRC}
    ${AUDIO_ENGINE_SRC}
    ${AUDIO_DRIVER_SRC}

    # Common public
    ${CMAKE_CURRENT_LIST_DIR}/iaudiodriver.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiodrivercontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/devtools/inputlag.h
    )

set(MODULE_LINK ${MODULE_LINK} ${AUDIO_ENGINE_LINK})

if (QT_ADD_CONCURRENT)
    list(APPEND MODULE_LINK Qt::Concurrent)
endif()

if (MUSE_MODULE_AUDIO_JACK)
    find_package(Jack REQUIRED)
    set(MODULE_INCLUDE_PRIVATE ${MODULE_INCLUDE_PRIVATE} ${JACK_INCLUDE_DIRS} )
    set(MODULE_LINK ${MODULE_LINK} ${JACK_LIBRARIES} pthread )
    find_package(ALSA REQUIRED)
    set(MODULE_INCLUDE_PRIVATE ${MODULE_INCLUDE_PRIVATE} ${ALSA_INCLUDE_DIRS} )
    set(MODULE_LINK ${MODULE_LINK} ${ALSA_LIBRARIES} pthread )
else ()
    if (OS_IS_MAC)
        find_library(AudioToolbox NAMES AudioToolbox)
        find_library(CoreAudio NAMES CoreAudio)
        set(MODULE_LINK ${MODULE_LINK} ${AudioToolbox} ${CoreAudio})
    elseif (OS_IS_WIN)
        set(MODULE_LINK ${MODULE_LINK} winmm mmdevapi mfplat
            WindowsApp) # needed for C++/WinRT
    elseif (OS_IS_LIN)
        find_package(ALSA REQUIRED)
        set(MODULE_INCLUDE_PRIVATE ${MODULE_INCLUDE_PRIVATE} ${ALSA_INCLUDE_DIRS} )
        set(MODULE_LINK ${MODULE_LINK} ${ALSA_LIBRARIES} pthread )
        if (MUSE_PIPEWIRE_AUDIO_DRIVER)
            find_package(PkgConfig)
            pkg_check_modules(PipeWire libpipewire-0.3)
            if (PipeWire_FOUND)
                message(STATUS "Found Pipewire: ${PipeWire_LIBRARIES}")
                set(MODULE_SRC
                    ${MODULE_SRC}
                    ${PW_AUDIO_DRIVER_SRC}
                )
                set(MODULE_INCLUDE ${MODULE_INCLUDE} ${PipeWire_INCLUDE_DIRS} )
                set(MODULE_LINK ${MODULE_LINK} ${PipeWire_LIBRARIES})
                set(MODULE_DEF ${MODULE_DEF} -DMUSE_PIPEWIRE_AUDIO_DRIVER)
            else()
                message(WARNING "Pipewire development files not found.\nPipewire support is disabled from the build.")
            endif()
        endif()
    endif()
endif()

# Adding driver source
set(MODULE_SRC
    ${MODULE_SRC}
    ${DRIVER_SRC}
)

set(MODULE_INCLUDE ${MODULE_INCLUDE}
    ${AUDIO_DRIVER_INC}
)

set(MODULE_LINK ${MODULE_LINK}
    ${AUDIO_DRIVER_LINK}
)

setup_module()

if (MUSE_MODULE_AUDIO_TESTS)
    add_subdirectory(tests)
endif()
