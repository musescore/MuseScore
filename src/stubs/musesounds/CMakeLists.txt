# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited
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

muse_create_module(musesounds STUB)

target_sources(musesounds PRIVATE
    musesoundsstubmodule.cpp
    musesoundsstubmodule.h
    musesoundscheckupdatescenariostub.cpp
    musesoundscheckupdatescenariostub.h
    musesoundscheckupdateservicestub.cpp
    musesoundscheckupdateservicestub.h
    musesamplercheckupdatescenariostub.cpp
    musesamplercheckupdatescenariostub.h
    musesamplercheckupdateservicestub.cpp
    musesamplercheckupdateservicestub.h
)

if (MUE_BUILD_MUSESOUNDS_QML)
    add_subdirectory(qml/MuseScore/MuseSounds)
endif()
