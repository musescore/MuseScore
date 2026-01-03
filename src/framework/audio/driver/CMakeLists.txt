# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(muse_audio_driver)

target_include_directories(muse_audio_driver PRIVATE ..)

if (OS_IS_WIN)
    target_sources(muse_audio_driver PRIVATE
        platform/win/wasapiaudiodriver.cpp
        platform/win/wasapiaudiodriver.h
    )

    target_link_libraries(muse_audio_driver PRIVATE
        winmm
        mmdevapi
        mfplat
        WindowsApp # needed for C++/WinRT
    )

    if (MUSE_MODULE_AUDIO_ASIO)
        add_subdirectory(platform/win/asio/asiosdk asiosdk)

        target_sources(muse_audio_driver PRIVATE
            platform/win/asio/asioaudiodriver.cpp
            platform/win/asio/asioaudiodriver.h
        )

        target_link_libraries(muse_audio_driver PRIVATE asiosdk)
    endif()
elseif(OS_IS_LIN OR OS_IS_FBSD)
    target_sources(muse_audio_driver PRIVATE
        platform/lin/alsaaudiodriver.cpp
        platform/lin/alsaaudiodriver.h
        platform/lin/audiodeviceslistener.cpp
        platform/lin/audiodeviceslistener.h
    )
    find_package(ALSA REQUIRED)
    target_link_libraries(muse_audio_driver PRIVATE ALSA::ALSA pthread)

    if (MUSE_MODULE_AUDIO_PIPEWIRE)
        find_package(PkgConfig)
        pkg_check_modules(PipeWire libpipewire-0.3)
        if (PipeWire_FOUND)
            message(STATUS "Found Pipewire: ${PipeWire_LIBRARIES}")
            target_sources(muse_audio_driver PRIVATE
                platform/lin/pwaudiodriver.cpp
                platform/lin/pwaudiodriver.h
            )
            target_include_directories(muse_audio_driver PRIVATE ${PipeWire_INCLUDE_DIRS})
            target_link_libraries(muse_audio_driver PRIVATE ${PipeWire_LIBRARIES})
        else()
            message(WARNING "Pipewire development files not found.\nPipewire support is disabled from the build.")
        endif()
    endif()

    if (MUSE_MODULE_AUDIO_JACK)
        target_sources(muse_audio_driver PRIVATE
            platform/jack/jackaudiodriver.cpp
            platform/jack/jackaudiodriver.h
        )

        find_package(Jack REQUIRED)
        target_include_directories(muse_audio_driver PRIVATE ${JACK_INCLUDE_DIRS})
        target_link_libraries(muse_audio_driver PRIVATE ${JACK_LIBRARIES} pthread)
    endif()
elseif(OS_IS_MAC)
    target_sources(muse_audio_driver PRIVATE
        platform/osx/osxaudiodriver.mm
        platform/osx/osxaudiodriver.h
    )

    set_source_files_properties(
        platform/osx/osxaudiodriver.mm
        PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
        SKIP_PRECOMPILE_HEADERS ON
    )

    find_library(AudioToolbox NAMES AudioToolbox)
    find_library(CoreAudio NAMES CoreAudio)
    target_link_libraries(muse_audio_driver PRIVATE ${AudioToolbox} ${CoreAudio})
elseif(OS_IS_WASM)
    target_sources(muse_audio_driver PRIVATE
        platform/web/webaudiodriver.cpp
        platform/web/webaudiodriver.h
        platform/web/webaudiochannel.cpp
        platform/web/webaudiochannel.h
    )
endif()

target_link_libraries(muse_audio_driver PRIVATE muse_audio_common)
