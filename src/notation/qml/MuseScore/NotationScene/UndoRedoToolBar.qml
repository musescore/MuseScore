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

Rectangle {
    id: root

    property alias navigation: navPanel

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        model.load()
    }

    NavigationPanel {
        id: navPanel
        name: "UndoRedoToolBar"
        enabled: root.enabled && root.visible
    }

    UndoRedoModel {
        id: model
    }

    Row {
        anchors.centerIn: parent

        height: childrenRect.height
        spacing: 2

        FlatButton {
            icon: model.undoItem.icon
            iconFont: ui.theme.toolbarIconsFont

            toolTipTitle: model.undoItem.title
            toolTipDescription: model.undoItem.description
            toolTipShortcut: model.undoItem.shortcut

            enabled: model.undoItem.enabled
            normalStateColor: "transparent"

            navigation.panel: navPanel
            navigation.order: 1

            onClicked: {
                model.undo()
            }
        }

        FlatButton {
            icon: model.redoItem.icon
            iconFont: ui.theme.toolbarIconsFont

            toolTipTitle: model.redoItem.title
            toolTipDescription: model.redoItem.description
            toolTipShortcut: model.redoItem.shortcut

            enabled: model.redoItem.enabled
            normalStateColor: "transparent"

            navigation.panel: navPanel
            navigation.order: 2

            onClicked: {
                model.redo()
            }
        }
    }
}
