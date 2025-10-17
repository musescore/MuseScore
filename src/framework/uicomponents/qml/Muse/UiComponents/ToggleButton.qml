/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

FocusScope {
    id: root

    property alias navigation: navCtrl

    property bool checked: false

    property alias text: label.text

    property string toolTipTitle: ""
    property string toolTipDescription: ""
    property string toolTipShortcut: ""

    signal toggled

    implicitWidth: contentRow.implicitWidth
    implicitHeight: contentRow.implicitHeight

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "ToggleButton"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.CheckBox
        accessible.name: root.text
        accessible.checked: root.checked

        onActiveChanged: {
            if (!root.activeFocus) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.toggled()
    }

    RowLayout {
        id: contentRow
        width: Math.min(implicitWidth, parent.width)
        spacing: 6

        Rectangle {
            id: buttonBackground

            implicitHeight: 20
            implicitWidth: 36

            color: root.checked ? ui.theme.accentColor : ui.theme.buttonColor

            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor
            radius: root.height / 2

            NavigationFocusBorder {
                navigationCtrl: navCtrl
            }

            StyledRectangularShadow {
                anchors.fill: buttonHandle
                offset.y: 1
                blur: 4
                radius: buttonHandle.radius
            }

            Rectangle {
                id: buttonHandle

                readonly property int margins: 2

                anchors.verticalCenter: parent.verticalCenter
                x: root.checked ? parent.width - width - margins : margins
                width: height
                height: parent.height - margins * 2

                radius: width / 2
                color: "#FFFFFF"

                Behavior on x {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.OutCubic
                    }
                }
            }
        }

        StyledTextLabel {
            id: label
            visible: !isEmpty

            Layout.fillWidth: true

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: contentRow

        enabled: root.enabled
        hoverEnabled: !label.hoveredLink
        z: label.z - 1 // enable clicking on links in label text

        onClicked: {
            navigation.requestActiveByInteraction()

            root.toggled()
        }

        onPressed: {
            ui.tooltip.hide(root, true)
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
