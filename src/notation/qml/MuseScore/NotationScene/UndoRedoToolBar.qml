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


    width: content.width
    height: content.height

    Component.onCompleted: {
        model.load()
    }

    property NavigationPanel navigationPanel: NavigationPanel {
        id: navPanel
        name: "UndoRedoToolBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("notation", "Undo redo toolbar")
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

            property var item: Boolean(model) ? model.undoItem : null

            icon: Boolean(item) ? item.icon : IconCode.NONE
            iconFont: ui.theme.toolbarIconsFont

            toolTipTitle: Boolean(item) ? item.title : ""
            toolTipDescription: Boolean(item) ? item.description : ""
            toolTipShortcut: Boolean(item) ? item.shortcuts : ""

            enabled: Boolean(item) ? item.enabled : false
            transparent: true

            navigation.panel: root.navigationPanel
            navigation.order: 1

            onClicked: {
                model.undo()
            }
        }

        FlatButton {
            width: 30
            height: width

            property var item: Boolean(model) ? model.redoItem : null

            icon: Boolean(item) ? item.icon : IconCode.NONE
            iconFont: ui.theme.toolbarIconsFont

            toolTipTitle: Boolean(item) ? item.title : ""
            toolTipDescription: Boolean(item) ? item.description : ""
            toolTipShortcut: Boolean(item) ? item.shortcuts : ""

            enabled: Boolean(item) ? item.enabled : false
            transparent: true

            navigation.panel: root.navigationPanel
            navigation.order: 2

            onClicked: {
                model.redo()
            }
        }
    }
}
