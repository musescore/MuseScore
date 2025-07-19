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

# should be first to work pch
if (MUSE_MODULE_GLOBAL)
    add_subdirectory(global)
endif()

if (MUSE_MODULE_ACCESSIBILITY)
    add_subdirectory(accessibility)
endif()

if (MUSE_MODULE_ACTIONS)
    add_subdirectory(actions)
endif()

if (MUSE_MODULE_AUDIO)
    add_subdirectory(audio)
endif()

if (MUSE_MODULE_AUDIOPLUGINS)
    add_subdirectory(audioplugins)
endif()

if (MUSE_MODULE_AUTOBOT)
    add_subdirectory(autobot)
endif()

if (MUSE_MODULE_CLOUD)
    add_subdirectory(cloud)
endif()

if (MUSE_MODULE_DIAGNOSTICS)
    add_subdirectory(diagnostics)
endif()

if (MUSE_MODULE_DRAW)
    add_subdirectory(draw)
endif()

if (MUSE_MODULE_MIDI)
    add_subdirectory(midi)
endif()

if (MUSE_MODULE_MPE)
    add_subdirectory(mpe)
endif()

if (MUSE_MODULE_MUSESAMPLER)
    add_subdirectory(musesampler)
endif()

if (MUSE_MODULE_UI)
    add_subdirectory(ui)
    add_subdirectory(uicomponents)
endif()

if (MUSE_MODULE_DOCKWINDOW)
    add_subdirectory(dockwindow)
endif()

if (MUSE_MODULE_NETWORK)
    add_subdirectory(network)
endif()

if (MUSE_MODULE_SHORTCUTS)
    add_subdirectory(shortcuts)
endif()

if (MUSE_MODULE_TOURS)
    add_subdirectory(tours)
endif()

if (MUSE_MODULE_MULTIINSTANCES)
    add_subdirectory(multiinstances)
endif()

if (MUSE_MODULE_LANGUAGES)
    add_subdirectory(languages)
endif()

if (MUSE_MODULE_LEARN)
    add_subdirectory(learn)
endif()

if (MUSE_MODULE_EXTENSIONS)
    add_subdirectory(extensions)
endif()

if (MUSE_MODULE_WORKSPACE)
    add_subdirectory(workspace)
endif()

if (MUSE_MODULE_VST)
    add_subdirectory(vst)
endif()

if (MUSE_MODULE_UPDATE)
    add_subdirectory(update)
endif()

# Tests
if (MUSE_ENABLE_UNIT_TESTS)
    define_property(TARGET PROPERTY OUTPUT_XML
        BRIEF_DOCS "List XML files outputted by google test."
        FULL_DOCS "List XML files outputted by google test."
    )

    set(INSTALL_GTEST OFF)
    add_subdirectory(testing/thirdparty/googletest)
endif()

# Stubs
add_subdirectory(stubs)



