/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15

import Muse.UiComponents 1.0

import MuseScore.NotationScene 1.0

StyledToolBarView {
    id: root

    navigationPanel.name: "UndoRedoToolBar"
    navigationPanel.accessible.name: qsTrc("notation", "Undo redo toolbar")

    spacing: 0
    padding: 6

    model: UndoRedoToolbarModel { }

    sourceComponentCallback: function(type) {
        switch(type) {
        case ToolBarItemType.ACTION: return controlComp
        }

        return null
    }

    Component {
        id: controlComp

        StyledToolBarItem {
            width: 30
            height: width

            navigation.panel: root.navigationPanel
            navigation.order: model.index
        }
    }
}
