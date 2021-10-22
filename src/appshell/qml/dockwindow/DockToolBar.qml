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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Dock 1.0

DockToolBarView {
    id: root

    default property alias contentComponent: contentLoader.sourceComponent

    property alias floatable: gripButton.visible

    property int padding: 2
    property int spacing: 2

    contentWidth: content.width
    contentHeight: content.height

    minimumWidth: Math.min(contentWidth, maximumWidth)
    minimumHeight: Math.min(contentHeight, maximumHeight)

    QtObject {
        id: prv

        readonly property int minimumLength: 48
        readonly property int maximumLength: 9999999
        readonly property int gripButtonWidth: gripButton.visible ? gripButton.width : 0
        readonly property int gripButtonHeight: gripButton.visible ? gripButton.height : 0
    }

    Rectangle {
        id: contentBackground

        color: ui.theme.backgroundPrimaryColor

        function printInfo() {
            console.debug("------------------------------")
            console.debug("obj: " + objectName)
            console.debug("contentSize: " + Qt.size(contentLoader.width, contentLoader.height))
            console.debug("bakgroundSize: " + Qt.size(contentBackground.width, contentBackground.height))
            console.debug("------------------------------\n")
        }

        Component.onCompleted: {
            Qt.callLater(printInfo)
        }

        Item {
            id: content

            width: childrenRect.width
            height: childrenRect.height

            FlatButton {
                id: gripButton

                mouseArea.objectName: root.objectName + "_gripButton"

                transparent: true
                icon: IconCode.TOOLBAR_GRIP

                Component.onCompleted: {
                    root.setDraggableMouseArea(mouseArea)
                }
            }

            Loader {
                id: contentLoader
            }
        }
    }

    states: [
        State {
            name: "HORIZONTAL"
            when: root.orientation === Qt.Horizontal

            PropertyChanges {
                target: root

                maximumWidth: prv.maximumLength
                maximumHeight: prv.minimumLength
            }

            PropertyChanges {
                target: gripButton

                anchors.left: parent.left
                anchors.leftMargin: root.padding
                anchors.top: undefined

                x: 0
                y: (contentBackground.height - gripButton.height) / 2
            }

            PropertyChanges {
                target: contentLoader

                anchors.left: gripButton.visible ? gripButton.right : parent.left
                anchors.leftMargin: gripButton.visible ? root.spacing : root.padding
                anchors.top: undefined

                x: 0
                y: (contentBackground.height - contentLoader.height) / 2
            }
        },

        State {
            name: "VERTICAL"
            when: root.orientation === Qt.Vertical

            PropertyChanges {
                target: root

                maximumWidth: content.width
                maximumHeight: prv.maximumLength
            }

            PropertyChanges {
                target: gripButton

                anchors.top: parent.top
                anchors.topMargin: root.padding
                anchors.left: undefined

                x: (contentBackground.width - gripButton.width) / 2
                y: 0

                rotation: 90
            }

            PropertyChanges {
                target: contentLoader

                anchors.top: gripButton.visible ? gripButton.bottom : parent.top
                anchors.topMargin: gripButton.visible ? root.spacing : root.padding
                anchors.left: undefined

                x: (contentBackground.width - contentLoader.width) / 2
                y: 0
            }
        }
    ]
}
