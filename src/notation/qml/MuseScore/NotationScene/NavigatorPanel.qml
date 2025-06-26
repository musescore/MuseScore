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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

Item {
    id: root

    property bool isFloating: false
    signal requestDockToNavigatorZone()

    width: 400
    height: 120

    Row {
        id: titleBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 36
        spacing: 8

        StyledTextLabel {
            text: qsTr("Navigator")
            font.pixelSize: 16
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            color: ui.theme.fontPrimaryColor
            anchors.verticalCenter: parent.verticalCenter
        }

        Item { Layout.fillWidth: true }

        MenuButton {
            id: menuButton
            icon: IconCode.MORE_VERT
            anchors.verticalCenter: parent.verticalCenter

            Menu {
                id: navigatorMenu

                MenuItem {
                    text: qsTr("Close")
                    onTriggered: root.visible = false
                }
                MenuItem {
                    text: root.isFloating ? qsTr("Dock") : qsTr("Undock")
                    onTriggered: {
                        if (root.isFloating) {
                            root.requestDockToNavigatorZone()
                            root.isFloating = false
                        } else {
                            if (root.parent) {
                                root.parent.floating = true
                            }
                            root.isFloating = true
                        }
                    }
                }
            }
        }
    }

    NotationNavigator {
        id: navigator
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
