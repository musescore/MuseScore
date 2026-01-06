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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

Rectangle {
    id: root

    color: ui.theme.backgroundSecondaryColor

    property string lastClickedInfo: ""

    signal activeFocusRequested()

    Rectangle {
        id: infoPanel

        anchors.left: parent.left
        anchors.right: parent.right
        height: 64

        StyledTextLabel {
            anchors.fill: parent
            anchors.margins: 8
            verticalAlignment: Text.AlignVCenter
            text: "Last clicked: " + root.lastClickedInfo
            color: "#000000"
        }
    }

    KeyNavSection {
        id: mainMenu
        anchors.top: infoPanel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 64
        color: "#729fcf"

        anchors.margins: ui.theme.navCtrlBorderWidth

        sectionName: "mainMenu"
        sectionOrder: 101

        onActiveChanged: {
            if (active) {
                root.activeFocusRequested()
                root.forceActiveFocus()
            }
        }

        RowLayout {
            anchors.fill: parent
            spacing: 8

            Repeater {
                model: 3

                KeyNavSubSection {
                    required property int index

                    Layout.fillWidth: true
                    keynavSection: mainMenu.keynavSection
                    subsectionName: "subsec" + index
                    subsectionOrder: index
                    color: mainMenu.color
                    onClicked: function(info) {
                        root.lastClickedInfo = "sec: " + mainMenu.sectionName + ", " + info
                    }
                }
            }
        }
    }

    KeyNavSection {
        id: topTools
        anchors.top: mainMenu.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 64
        color: "#ef2929"

        anchors.margins: ui.theme.navCtrlBorderWidth

        sectionName: "topTools"
        sectionOrder: 102

        Row {
            anchors.fill: parent
            spacing: 8

            Repeater {
                model: 2

                KeyNavSubSection {
                    required property int index

                    anchors.verticalCenter: parent.verticalCenter
                    keynavSection: topTools.keynavSection
                    subsectionName: "subsec" + index
                    subsectionOrder: index
                    color: topTools.color
                    onClicked: function(info) {
                        root.lastClickedInfo = "sec: " + topTools.sectionName + ", " + info
                    }
                }
            }
        }
    }

    KeyNavSection {
        id: leftPanel
        anchors.left: parent.left
        anchors.top: topTools.bottom
        anchors.bottom: parent.bottom
        width: 120
        color: "#729fcf"

        anchors.margins: ui.theme.navCtrlBorderWidth

        sectionName: "leftPanel"
        sectionOrder: 103

        Column {
            anchors.fill: parent
            spacing: 8

            Repeater {
                model: 2

                KeyNavSubSection {
                    required property int index

                    keynavSection: leftPanel.keynavSection
                    subsectionName: "subsec" + index
                    subsectionOrder: index
                    color: leftPanel.color
                    onClicked: function(info) {
                        root.lastClickedInfo = "sec: " + leftPanel.sectionName + ", " + info
                    }
                }
            }
        }
    }

    KeyNavSection {
        id: rightPanel
        anchors.right: parent.right
        anchors.top: topTools.bottom
        anchors.bottom: parent.bottom
        width: 120
        color: "#8ae234"

        anchors.margins: ui.theme.navCtrlBorderWidth

        sectionName: "rightPanel"
        sectionOrder: 105

        Column {
            anchors.fill: parent
            spacing: 8

            Repeater {
                model: 2

                KeyNavSubSection {
                    required property int index

                    keynavSection: rightPanel.keynavSection
                    subsectionName: "subsec" + index
                    subsectionOrder: index
                    color: rightPanel.color
                    onClicked: function(info) {
                        root.lastClickedInfo = "sec: " + rightPanel.sectionName + ", " + info
                    }
                }
            }
        }
    }

    KeyNavSection {
        id: centerPanel
        anchors.left: leftPanel.right
        anchors.right: rightPanel.left
        anchors.top: topTools.bottom
        anchors.bottom: parent.bottom
        color: "#ef2929"

        anchors.margins: ui.theme.navCtrlBorderWidth

        sectionName: "centerPanel"
        sectionOrder: 104

        KeyNavSubSection {
            keynavSection: centerPanel.keynavSection
            subsectionName: "subsec0"
            subsectionOrder: 0
            color: centerPanel.color
            onClicked: function(info) {
                root.lastClickedInfo = "sec: " + centerPanel.sectionName + ", " + info
            }
        }
    }
}
