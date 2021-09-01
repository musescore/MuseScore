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

import MuseScore.UiComponents 1.0

RadioButton {
    id: root

    default property Component contentComponent

    implicitHeight: 20
    implicitWidth: ListView.view ? (ListView.view.width - (ListView.view.spacing * (ListView.view.count - 1))) / ListView.view.count
                                 : 30
    spacing: 6
    padding: 0

    font: ui.theme.bodyFont

    hoverEnabled: true

    contentItem: Item {
        anchors.fill: parent
        anchors.leftMargin: root.indicator.width + root.spacing + root.leftPadding

        Loader {
            id: contentLoader

            anchors.fill: parent

            sourceComponent: Boolean(root.contentComponent) ? root.contentComponent : textLabel

            Component {
                id: textLabel

                StyledTextLabel {
                    text: root.text
                    font: root.font
                    horizontalAlignment: Qt.AlignLeft
                }
            }
        }
    }

    indicator: Item {
        x: root.leftPadding
        y: Boolean(parent) ? parent.height / 2 - height / 2 : 0
        implicitWidth: 20
        implicitHeight: implicitWidth

        Rectangle {
            id: backgroundRect
            anchors.centerIn: parent
            width: 2 * radius
            height: width
            radius: 10

            property real borderColorOpacity: ui.theme.buttonOpacityNormal

            color: ui.theme.textFieldColor
            border.color: Utils.colorWithAlpha(ui.theme.fontPrimaryColor, borderColorOpacity)
            border.width: 1
        }

        Rectangle {
            id: highlightRect
            anchors.centerIn: parent
            width: 2 * radius
            height: width
            radius: 5

            color: ui.theme.accentColor
            visible: root.checked
        }
    }

    states: [
        State {
            name: "PRESSED"
            when: root.pressed

            PropertyChanges {
                target: backgroundRect
                borderColorOpacity: ui.theme.buttonOpacityHit
            }

            PropertyChanges {
                target: highlightRect
                visible: true
            }
        },

        State {
            name: "SELECTED"
            when: root.checked && !root.hovered

            PropertyChanges {
                target: backgroundRect
                borderColorOpacity: ui.theme.buttonOpacityNormal
            }

            PropertyChanges {
                target: highlightRect
                visible: true
            }
        },

        State {
            name: "HOVERED"
            when: root.hovered && !root.checked && !root.pressed

            PropertyChanges {
                target: backgroundRect
                borderColorOpacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "SELECTED_HOVERED"
            when: root.hovered && root.checked

            PropertyChanges {
                target: backgroundRect
                borderColorOpacity: ui.theme.buttonOpacityHover
            }

            PropertyChanges {
                target: highlightRect
                visible: true
            }
        }
    ]
}
