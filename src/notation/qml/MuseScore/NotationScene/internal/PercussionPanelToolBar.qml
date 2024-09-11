/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import MuseScore.NotationScene 1.0

Item {
    id: root

    required property var model
    required property int panelWidth

    property alias navigation: navPanel

    anchors.fill: parent

    QtObject {
        id: prv
        // The central buttons are placed centrally relative to the panel, not the toolbar area
        readonly property int tabAreaLength: root.panelWidth - root.width
        readonly property int centerX: (root.panelWidth / 2) - prv.tabAreaLength

        readonly property int spacing: 4
    }

    NavigationPanel {
        id: navPanel
        name: "PercussionPanelToolBar"
        enabled: root.enabled && root.visible
    }

    Row {
        id: centralButtonsRow

        anchors.verticalCenter: parent.verticalCenter
        x: prv.centerX - (centralButtonsRow.width / 2)

        visible: model.currentPanelMode !== PanelMode.EDIT_LAYOUT

        FlatButton {
            icon: IconCode.EDIT
            text: qsTrc("notation", "Write")
            orientation: Qt.Horizontal
            accentButton: model.currentPanelMode === PanelMode.WRITE

            navigation.panel: navPanel
            navigation.row: 0

            onClicked: {
                root.model.currentPanelMode = PanelMode.WRITE
            }
        }

        FlatButton {
            icon: IconCode.PLAY
            text: qsTrc("notation", "Preview")
            orientation: Qt.Horizontal
            accentButton: model.currentPanelMode === PanelMode.SOUND_PREVIEW

            navigation.panel: navPanel
            navigation.row: 0

            onClicked: {
                root.model.currentPanelMode = PanelMode.SOUND_PREVIEW
            }
        }
    }

    FlatButton {
        id: finishEditingButton

        anchors.verticalCenter: parent.verticalCenter
        x: prv.centerX - (finishEditingButton.width / 2)

        visible: model.currentPanelMode === PanelMode.EDIT_LAYOUT
        text: qsTrc("notation", "Finish editing")
        orientation: Qt.Horizontal
        accentButton: true

        navigation.panel: navPanel
        navigation.row: 0

        onClicked: {
            root.model.finishEditing()
        }
    }


    Row {
        id: rightSideButtonsRow

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: prv.spacing

        spacing: prv.spacing

        FlatButton {
            icon: IconCode.SPLIT_VIEW_HORIZONTAL
            text: qsTrc("notation", "Layout")
            orientation: Qt.Horizontal

            navigation.panel: navPanel
            navigation.row: 0

            onClicked: {
                menuLoader.toggleOpened(root.model.layoutMenuItems)
            }

            StyledMenuLoader {
                id: menuLoader

                onHandleMenuItem: function(itemId) {
                    root.model.handleMenuItem(itemId)
                }
            }
        }

        FlatButton {
            enabled: model.currentPanelMode !== PanelMode.EDIT_LAYOUT
            text: qsTrc("notation", "Customize kit")
            orientation: Qt.Horizontal

            navigation.panel: navPanel
            navigation.row: 0

            onClicked: {
                api.launcher.open("muse://devtools/interactive/sample")
            }
        }
    }
}
