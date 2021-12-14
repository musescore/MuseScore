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

    property int icon: IconCode.NONE
    property string text: ""
    property int textFormat: Text.AutoText

    property string toolTipTitle: ""
    property string toolTipDescription: ""
    property string toolTipShortcut: ""

    property font iconFont: ui.theme.iconsFont
    property font textFont: ui.theme.bodyFont

    property bool transparent: false
    property bool accentButton: false

    property color normalColor:
        transparent ? "transparent" : accentButton ? accentColor : ui.theme.buttonColor
    property color hoverHitColor: accentButton ? accentColor : ui.theme.buttonColor
    property color accentColor: ui.theme.accentColor

    property bool narrowMargins: false
    property real margins: narrowMargins ? 12 : 16
    property real minWidth: narrowMargins ? 24 : 132

    property bool drawFocusBorderInsideRect: false

    property int orientation: Qt.Vertical
    readonly property bool isVertical: root.orientation === Qt.Vertical

    property alias navigation: navCtrl
    property alias accessible: navCtrl.accessible

    property alias mouseArea: mouseArea

    property bool isClickOnKeyNavTriggered: true

    property Component contentItem: defaultContentComponent
    property Component backgroundItem: defaultBackgroundComponent

    signal clicked(var mouse)
    signal pressAndHold(var mouse)

    objectName: root.text

    height: contentLoader.item.height + 14
    width: Boolean(text) ? Math.max(contentLoader.item.width + 2 * margins, root.isVertical ? minWidth : 0)
                         : contentLoader.item.width + 16

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    NavigationControl {
        id: navCtrl
        name: root.objectName !== "" ? root.objectName : "FlatButton"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.Button
        accessible.name: Boolean(root.text) ? root.text : root.toolTipTitle
        accessible.description: root.toolTipDescription
        accessible.visualItem: root

        onTriggered: {
            if (navCtrl.enabled && root.isClickOnKeyNavTriggered) {
                root.clicked(null)
            }
        }
    }

    Loader {
        anchors.fill: parent

        sourceComponent: root.backgroundItem
    }

    Component {
        id: defaultBackgroundComponent

        Rectangle {
            id: background

            color: root.normalColor
            opacity: ui.theme.buttonOpacityNormal

            radius: 3
            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor

            NavigationFocusBorder {
                navigationCtrl: navCtrl
                drawOutsideParent: !root.drawFocusBorderInsideRect
            }

            states: [
                State {
                    name: "PRESSED"
                    when: mouseArea.pressed

                    PropertyChanges {
                        target: background
                        color: root.hoverHitColor
                        opacity: ui.theme.buttonOpacityHit
                    }
                },

                State {
                    name: "HOVERED"
                    when: mouseArea.containsMouse && !mouseArea.pressed

                    PropertyChanges {
                        target: background
                        color: root.hoverHitColor
                        opacity: ui.theme.buttonOpacityHover
                    }
                }
            ]
        }
    }

    Loader {
        id: contentLoader

        anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

        sourceComponent: root.contentItem
    }

    Component {
        id: defaultContentComponent

        Item {
            id: contentWrapper

            property int spacing: Boolean(!buttonIcon.isEmpty) && Boolean(textLabel.text) ? 4 : 0

            height: !root.isVertical ? Math.max(buttonIcon.height, textLabel.height) : buttonIcon.height + textLabel.height + spacing
            width: root.isVertical ? Math.max(textLabel.width, buttonIcon.width) : buttonIcon.width + textLabel.width + spacing

            StyledIconLabel {
                id: buttonIcon

                iconCode: root.icon
                font: root.iconFont
            }

            StyledTextLabel {
                id: textLabel

                text: root.text
                font: root.textFont
                textFormat: root.textFormat

                height: text === "" ? 0 : implicitHeight
                horizontalAlignment: Text.AlignHCenter
            }

            states: [
                State {
                    when: !root.isVertical
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
                    when: root.isVertical
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
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        hoverEnabled: true

        onClicked: function(mouse) {
            root.clicked(mouse)
        }

        onPressed: {
            ui.tooltip.hide(root)
        }

        onPressAndHold: function(mouse) {
            root.pressAndHold(mouse)
        }

        onContainsMouseChanged: {
            if (!Boolean(root.toolTipTitle)) {
                return
            }

            if (mouseArea.containsMouse) {
                ui.tooltip.show(root, root.toolTipTitle, root.toolTipDescription, root.toolTipShortcut)
            } else {
                ui.tooltip.hide(root)
            }
        }
    }
}
