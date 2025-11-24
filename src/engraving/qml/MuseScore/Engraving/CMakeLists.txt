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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

if (NOT MUE_BUILD_ENGRAVING_DEVTOOLS)
    # Currently no non-devtools QML files
    return()
endif()

muse_create_qml_module(engraving_qml FOR engraving)

qt_add_qml_module(engraving_qml
    URI MuseScore.Engraving
    VERSION 1.0
    SOURCES
        devtools/corruptscoredevtoolsmodel.cpp
        devtools/corruptscoredevtoolsmodel.h
        devtools/engravingelementsmodel.cpp
        devtools/engravingelementsmodel.h
        devtools/engravingstylemodel.cpp
        devtools/engravingstylemodel.h
        devtools/engravingundostackmodel.cpp
        devtools/engravingundostackmodel.h
    QML_FILES
        devtools/EngravingElementsDialog.qml
        devtools/EngravingElementsPanel.qml
        devtools/EngravingStyleDialog.qml
        devtools/EngravingStylePanel.qml
        devtools/EngravingUndoStackDialog.qml
        devtools/EngravingUndoStackPanel.qml
    IMPORTS
        TARGET muse_uicomponents_qml
)

# Necessary for the auto-generated sources
target_include_directories(engraving_qml PRIVATE devtools)
