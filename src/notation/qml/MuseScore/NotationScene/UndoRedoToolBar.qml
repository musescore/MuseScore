/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.NotationScene 1.0

Item {
    id: root

    property alias navigation: navPanel

    width: content.width
    height: content.height

    Component.onCompleted: {
        model.load()
    }

    NavigationPanel {
        id: navPanel
        name: "UndoRedoToolBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("notation", "Undo Redo toolbar")
    }

    UndoRedoModel {
        id: model
    }

    Row {
        id: content

        readonly property int padding: 6

        //! NOTE padding - 1 to compensate for the dock separator width.
        //! The separator is hidden, but the space for it is still allocated.
        //! That should be solved in KDDW.
        width: padding - 1 + childrenRect.width + padding
        height: childrenRect.height
        x: padding - 1

        spacing: 0

        FlatButton {
            width: 30
            height: width

            icon: model.undoItem.icon
            iconFont: ui.theme.toolbarIconsFont

            toolTipTitle: model.undoItem.title
            toolTipDescription: model.undoItem.description
            toolTipShortcut: model.undoItem.shortcut

            enabled: model.undoItem.enabled
            transparent: true

            navigation.panel: navPanel
            navigation.order: 1

            onClicked: {
                model.undo()
            }
        }

        FlatButton {
            width: 30
            height: width

            icon: model.redoItem.icon
            iconFont: ui.theme.toolbarIconsFont

            toolTipTitle: model.redoItem.title
            toolTipDescription: model.redoItem.description
            toolTipShortcut: model.redoItem.shortcut

            enabled: model.redoItem.enabled
            transparent: true

            navigation.panel: navPanel
            navigation.order: 2

            onClicked: {
                model.redo()
            }
        }
    }
}
