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
import QtQuick.Controls 2.0
import MuseScore.Ui 1.0

RadioDelegate {
    id: root

    default property Component contentComponent: null

    //! NOTE Don't use the `icon` property.
    //!      It's a property of the ancestor of RadioDelegate
    //!      and has the wrong type (QQuickIcon).
    //!      It can't be overridden either, as it is marked `FINAL`.
    property int iconCode: IconCode.NONE

    property alias radius: backgroundRect.radius

    property color normalStateColor: ui.theme.buttonColor
    property color hoverStateColor: ui.theme.buttonColor
    property color pressedStateColor: ui.theme.buttonColor
    property color selectedStateColor: ui.theme.accentColor

    property alias navigation: navCtrl

    implicitHeight: 30
    implicitWidth: ListView.view ? (ListView.view.width - (ListView.view.spacing * (ListView.view.count - 1))) / ListView.view.count
                                 : 30
    hoverEnabled: true

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    onClicked: root.ensureActiveFocus()

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "FlatRadioButton"

        accessible.role: MUAccessible.RadioButton
        accessible.name: root.text
        accessible.selected: root.checked

        onTriggered: root.checked = !root.checked
    }

    background: Rectangle {
        id: backgroundRect

        anchors.fill: parent

        NavigationFocusBorder { navigationCtrl: navCtrl }

        border.width: ui.theme.borderWidth
        border.color: ui.theme.strokeColor

        color: root.normalStateColor
        opacity: ui.theme.buttonOpacityNormal

        radius: 2
    }

    contentItem: Loader {
        id: contentLoader
        anchors.fill: parent

        sourceComponent: {
            if (root.contentComponent) {
                return root.contentComponent
            }

            if (root.iconCode && root.iconCode !== IconCode.NONE) {
                return iconComponent
            }

            return textComponent
        }

        Component {
            id: iconComponent

            StyledIconLabel {
                iconCode: root.iconCode
            }
        }

        Component {
            id: textComponent

            StyledTextLabel {
                text: root.text
            }
        }
    }

    indicator: Item {}

    states: [
        State {
            name: "HOVERED"
            when: root.hovered && !root.checked && !root.pressed

            PropertyChanges {
                target: backgroundRect
                color: root.hoverStateColor
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: root.pressed && !root.checked

            PropertyChanges {
                target: backgroundRect
                color: root.pressedStateColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "SELECTED"
            when: root.checked

            PropertyChanges {
                target: backgroundRect
                color: root.selectedStateColor
                opacity: ui.theme.buttonOpacityNormal
            }
        }
    ]
}
