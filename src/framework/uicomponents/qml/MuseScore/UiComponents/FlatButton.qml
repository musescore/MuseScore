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

FocusScope {
    id: root

    property alias icon: buttonIcon.iconCode
    property alias text: textLabel.text

    property string toolTipTitle: ""
    property string toolTipDescription: ""
    property string toolTipShortcut: ""

    property alias iconFont: buttonIcon.font
    property alias textFont: textLabel.font

    property color normalStateColor: prv.defaultColor
    property color hoveredStateColor: prv.defaultColor
    property color pressedStateColor: prv.defaultColor
    property bool accentButton: false

    property int orientation: Qt.Vertical

    property alias navigation: navCtrl
    property alias accessible: navCtrl.accessible

    property alias mouseArea: mouseArea

    property bool isClickOnKeyNavTriggered: true

    signal clicked(var mouse)
    signal pressAndHold(var mouse)

    objectName: root.text

    height: contentWrapper.height + 14
    width: (Boolean(text) ? Math.max(contentWrapper.width + 32, prv.isVertical ? 132 : 0) : contentWrapper.width + 16)

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    QtObject {
        id: prv

        property color defaultColor: root.accentButton ? ui.theme.accentColor : ui.theme.buttonColor
        property bool isVertical: root.orientation === Qt.Vertical
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName !== "" ? root.objectName : "FlatButton"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.Button
        accessible.name: root.text
        accessible.visualItem: root

        onTriggered: {
            if (root.isClickOnKeyNavTriggered) {
                root.clicked()
            }
        }
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: root.normalStateColor
        opacity: ui.theme.buttonOpacityNormal
        radius: 3
        border.width: navCtrl.active ? 2 : 0
        border.color: ui.theme.focusColor
    }

    Item {
        id: contentWrapper

        property int spacing: Boolean(!buttonIcon.isEmpty) && Boolean(textLabel.text) ? 4 : 0

        anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

        height: !prv.isVertical ? Math.max(buttonIcon.height, textLabel.height) : buttonIcon.height + textLabel.height + spacing
        width: prv.isVertical ? Math.max(textLabel.width, buttonIcon.width) : buttonIcon.width + textLabel.width + spacing

        StyledIconLabel {
            id: buttonIcon

            font.pixelSize: isEmpty ? 0 : ui.theme.iconsFont.pixelSize
        }

        StyledTextLabel {
            id: textLabel

            height: text === "" ? 0 : implicitHeight
            horizontalAlignment: Text.AlignHCenter
        }

        states: [
            State {
                when: !prv.isVertical
                AnchorChanges {
                    target: buttonIcon
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                }
                AnchorChanges {
                    target: textLabel
                    anchors.left: buttonIcon.right
                    anchors.verticalCenter: parent.verticalCenter
                }
                PropertyChanges {
                    target: textLabel
                    anchors.leftMargin: contentWrapper.spacing
                }
            },
            State {
                when: prv.isVertical
                AnchorChanges {
                    target: buttonIcon
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                AnchorChanges {
                    target: textLabel
                    anchors.top: buttonIcon.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                PropertyChanges {
                    target: textLabel
                    anchors.leftMargin: contentWrapper.spacing
                }
            }
        ]
    }

    states: [
        State {
            name: "PRESSED"
            when: mouseArea.pressed

            PropertyChanges {
                target: background
                color: root.pressedStateColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed

            PropertyChanges {
                target: background
                color: root.hoveredStateColor
                opacity: ui.theme.buttonOpacityHover
            }
        }
    ]

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        hoverEnabled: true

        onClicked: function (mouse) { 
            ui.tooltip.hide(this)
            root.clicked(mouse) 
        }
        
        onPressAndHold: function (mouse) { 
            ui.tooltip.hide(this)
            root.pressAndHold(mouse) 
        }

        onContainsMouseChanged: {
            if (!Boolean(root.toolTipTitle)) {
                return
            }

            if (mouseArea.containsMouse) {
                ui.tooltip.show(this, root.toolTipTitle, root.toolTipDescription, root.toolTipShortcut)
            } else {
                ui.tooltip.hide(this)
            }
        }
    }
}
