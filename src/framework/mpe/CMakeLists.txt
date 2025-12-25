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

muse_create_module(muse_mpe ALIAS muse::mpe)

target_sources(muse_mpe PRIVATE
    mpemodule.cpp
    mpemodule.h
    soundid.h
    mpetypes.h
    events.h
    playbacksetupdata.h
    iarticulationprofilesrepository.h

    internal/articulationstringutils.h
    internal/articulationprofilesrepository.cpp
    internal/articulationprofilesrepository.h
)

qt_add_resources(muse_mpe muse_mpe_resources
    PREFIX
        /mpe
    FILES
        resources/general_keyboard_articulations_profile.json
        resources/general_strings_articulations_profile.json
        resources/general_winds_articulations_profile.json
        resources/general_percussion_articulations_profile.json
        resources/general_voice_articulations_profile.json
)

if (MUSE_MODULE_MPE_TESTS)
    add_subdirectory(tests)
endif()

if (MUSE_MODULE_MPE_QML)
    add_subdirectory(qml/Muse/Mpe)
endif()
