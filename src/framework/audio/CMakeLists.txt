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

muse_create_module(muse_audio ALIAS muse::audio)

include(GetPlatformInfo)

if (OS_IS_WASM)
    set(WEBENGINE_FACADE_MODE ON)
endif()

target_sources(muse_audio PRIVATE
    # Common public
    iaudiodriver.h
    iaudiodrivercontroller.h
    devtools/inputlag.h
)

add_subdirectory(common)
add_subdirectory(main)
add_subdirectory(engine)
add_subdirectory(driver)

target_link_libraries(muse_audio PRIVATE muse_audio_main muse_audio_engine muse_audio_driver)

if (MUSE_MODULE_AUDIO_TESTS)
    add_subdirectory(tests)
endif()
