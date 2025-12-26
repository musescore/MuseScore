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

muse_create_module(notationscene)

target_sources(notationscene PRIVATE
    notationscenemodule.cpp
    notationscenemodule.h

    iselectinstrumentscenario.h

    internal/midiinputoutputcontroller.cpp
    internal/midiinputoutputcontroller.h
    internal/notationactioncontroller.cpp
    internal/notationactioncontroller.h
    internal/notationuiactions.cpp
    internal/notationuiactions.h

    utilities/engravingitempreviewpainter.cpp
    utilities/engravingitempreviewpainter.h
    utilities/percussionutilities.cpp
    utilities/percussionutilities.h
)

add_subdirectory(widgets)

target_link_libraries(notationscene PRIVATE
    notation
    muse::ui
    Qt::Quick
    Qt::StateMachine
    Qt::Widgets
)

if (MUE_BUILD_NOTATIONSCENE_QML)
    add_subdirectory(qml/MuseScore/NotationScene)
endif()
