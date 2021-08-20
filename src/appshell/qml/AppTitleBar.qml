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
import MuseScore.UiComponents 1.0
import MuseScore.AppShell 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    property alias title: titleLabel.text
    property rect titleMoveAreaRect: Qt.rect(titleMoveArea.x, titleMoveArea.y, titleMoveArea.width, titleMoveArea.height)

    signal showWindowMinimizedRequested()
    signal toggleWindowMaximizedRequested()
    signal closeWindowRequested()

    NavigationSection {
        id: navSec
        name: "AppTitleBar"
        order: 0
    }

    Item {
        anchors.fill: parent

        AppMenuBar {
            id: menu

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            navigation.section: navSec
            navigation.order: 0
        }

        Item {
            id: titleMoveArea

            anchors.top: parent.top
            anchors.left: menu.right
            anchors.right: systemButtons.left
            anchors.bottom: parent.bottom
        }

        StyledTextLabel {
            id: titleLabel
            anchors.centerIn: parent

            horizontalAlignment: Text.AlignLeft
            text: qsTrc("appshell", "MuseScore 4")

            textFormat: Text.RichText
        }

        Row {
            id: systemButtons

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            FlatButton {
                icon: IconCode.MINUS
                normalStateColor: "transparent"

                onClicked: {
                    root.showWindowMinimizedRequested()
                }
            }

            FlatButton {
                icon: IconCode.SPLIT_OUT_ARROWS
                normalStateColor: "transparent"

                onClicked: {
                    root.toggleWindowMaximizedRequested()
                }
            }

            FlatButton {
                icon: IconCode.CLOSE_X_ROUNDED
                normalStateColor: "transparent"
                pressedStateColor: "red"
                hoveredStateColor: "red"

                onClicked: {
                    root.closeWindowRequested()
                }
            }
        }
    }
}
