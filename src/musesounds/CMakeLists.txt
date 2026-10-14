# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited and others
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

muse_create_module(musesounds)

target_sources(musesounds PRIVATE
    musesoundsmodule.cpp
    musesoundsmodule.h
    imusesoundsrepository.h
    imusesoundsconfiguration.h
    musesoundstypes.h
    musesoundserrors.h
    imusesoundscheckupdatescenario.h
    imusesoundscheckupdateservice.h
    imusesamplercheckstatusservice.h
    imusesamplercheckstatusscenario.h

    internal/musesoundsrepository.cpp
    internal/musesoundsrepository.h
    internal/musesoundsconfiguration.cpp
    internal/musesoundsconfiguration.h

    internal/musesoundscheckupdatescenario.cpp
    internal/musesoundscheckupdatescenario.h
    internal/musesoundscheckupdateservice.cpp
    internal/musesoundscheckupdateservice.h
    internal/musesamplercheckstatusservice.cpp
    internal/musesamplercheckstatusservice.h
    internal/musesamplercheckstatusscenario.cpp
    internal/musesamplercheckstatusscenario.h
)

target_link_libraries(musesounds PRIVATE Qt::Concurrent)

add_subdirectory(qml/MuseScore/MuseSounds)

if (MUE_BUILD_MUSESOUNDS_TESTS)
    add_subdirectory(tests)
endif()
