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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(instrumentsscene)

target_sources(instrumentsscene PRIVATE
    instrumentsscenemodule.cpp
    instrumentsscenemodule.h

    internal/selectinstrumentscenario.cpp
    internal/selectinstrumentscenario.h
    internal/instrumentsuiactions.cpp
    internal/instrumentsuiactions.h
    internal/instrumentsactionscontroller.cpp
    internal/instrumentsactionscontroller.h
)

target_link_libraries(instrumentsscene PRIVATE engraving)

if (MUE_BUILD_INSTRUMENTSSCENE_QML)
    add_subdirectory(qml/MuseScore/InstrumentsScene)
endif()
