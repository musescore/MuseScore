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

import MuseScore.Ui 1.0
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    property alias navigation: keynavSub

    signal activeFocusRequested()

    width: view.width
    height: view.height

    Component.onCompleted: {
        toolbarModel.load()
    }

    NavigationPanel {
        id: keynavSub
        name: "NotationToolBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("notation", "Notation toolbar")
        onActiveChanged: function(active) {
            if (active) {
                root.activeFocusRequested()
                root.forceActiveFocus()
            }
        }
    }

    NotationToolBarModel {
        id: toolbarModel
    }

    ListView {
        id: view

        width: contentWidth
        height: contentItem.childrenRect.height

        orientation: Qt.Horizontal
        interactive: false
        spacing: 2

        model: toolbarModel

        delegate: FlatButton {
            id: btn

            height: 30

            property var item: Boolean(model) ? model.itemRole : null

            text: Boolean(item) ? item.title : ""
            icon: Boolean(item) ? item.icon : IconCode.NONE
            iconFont: ui.theme.toolbarIconsFont

            toolTipTitle: Boolean(item) ? item.title : ""
            toolTipDescription: Boolean(item) ? item.description : ""
            toolTipShortcut: Boolean(item) ? item.shortcuts : ""

            enabled: Boolean(item) ? item.enabled : false
            mouseArea.acceptedButtons: Qt.LeftButton | Qt.RightButton

            textFont: ui.theme.largeBodyFont

            navigation.panel: keynavSub
            navigation.name: toolTipTitle
            navigation.order: model.index

            transparent: true
            orientation: Qt.Horizontal

            ToolbarShortcutsContextMenu {
                id: contextMenu
                actionCode: btn.item ? btn.item.action : ""

                onItemHandled: {
                    console.log(btn.item.action + " handled!")
                }
            }

            onClicked: function(mouse) {
                if (mouse.button === Qt.LeftButton) {
                    toolbarModel.handleMenuItem(item.id)
                } else if (mouse.button === Qt.RightButton) {
                    contextMenu.show(Qt.point(mouseArea.mouseX, mouseArea.mouseY))
                }
            }
        }
    }
}
