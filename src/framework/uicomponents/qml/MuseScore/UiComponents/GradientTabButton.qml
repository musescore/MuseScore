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
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

RadioDelegate {
    id: root

    property Component iconComponent: null
    property string title: ""

    property int orientation: Qt.Vertical
    readonly property bool isVertical: orientation === Qt.Vertical

    property var normalStateFont: ui.theme.tabFont
    property var selectedStateFont: ui.theme.tabBoldFont

    property alias navigation: navCtrl

    height: 48

    spacing: 30
    leftPadding: 0
    rightPadding: 0

    //! NONE Disabled default Qt Accessible
    Accessible.ignored: true

    NavigationControl {
        id: navCtrl
        name: root.objectName
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.RadioButton
        accessible.name: root.title
        accessible.checked: root.checked

        onActiveChanged: {
            if (navCtrl.active) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.toggled()
    }

    background: Item {
        Rectangle {
            id: backgroundRect
            anchors.fill: parent

            color: ui.theme.backgroundPrimaryColor
            opacity: ui.theme.buttonOpacityNormal

            border.color: navCtrl.active ? ui.theme.fontPrimaryColor : ui.theme.strokeColor
            border.width: navCtrl.active ? ui.theme.navCtrlBorderWidth : ui.theme.borderWidth
        }

        Item {
            id: backgroundGradientRect
            anchors.fill: parent
            anchors.margins: backgroundRect.border.width

            visible: false

            LinearGradient {
                anchors.fill: parent
                start: Qt.point(0, 0)
                end: root.isVertical ? Qt.point(0, root.height) : Qt.point(root.width, 0)
                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: "transparent"
                    }
                    GradientStop {
                        position: 1.0
                        color: Utils.colorWithAlpha(ui.theme.accentColor, root.isVertical ? 0.2 : 0.1)
                    }
                }
            }

            Rectangle {
                id: line
                color: ui.theme.accentColor

                states: [
                    State {
                        when: root.isVertical
                        AnchorChanges {
                            target: line
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                        }

                        PropertyChanges {
                            target: line
                            height: 2
                        }
                    },
                    State {
                        when: !root.isVertical
                        AnchorChanges {
                            target: line
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                        }

                        PropertyChanges {
                            target: line
                            width: 2
                        }
                    }
                ]
            }
        }
    }

    contentItem: Row {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        spacing: root.spacing
        leftPadding: root.leftPadding
        rightPadding: root.rightPadding

        Loader {
            anchors.verticalCenter: parent.verticalCenter

            sourceComponent: root.iconComponent
            visible: Boolean(root.iconComponent)
        }

        StyledTextLabel {
            id: textLabel

            anchors.verticalCenter: parent.verticalCenter
            width: implicitWidth

            visible: Boolean(root.title)

            horizontalAlignment: Text.AlignLeft
            font: root.normalStateFont
            text: root.title
        }
    }

    indicator: Item {}

    states: [
        State {
            name: "HOVERED"
            when: root.hovered && !root.checked && !root.pressed

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: root.pressed && !root.checked

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "SELECTED"
            when: root.checked

            PropertyChanges {
                target: backgroundGradientRect
                visible: true
            }

            PropertyChanges {
                target: textLabel
                font: root.selectedStateFont
            }
        }
    ]
}
