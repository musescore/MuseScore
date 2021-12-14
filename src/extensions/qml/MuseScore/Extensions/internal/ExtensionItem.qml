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
import MuseScore.Extensions 1.0

Rectangle {
    id: root

    color: ui.theme.popupBackgroundColor
    border.color: ui.theme.fontPrimaryColor
    border.width: selected ? 2 : 0
    radius: 12

    property string code: ""
    property alias name: nameLabel.text
    property string description: ""
    property int status: 0
    property bool selected: false

    signal clicked(string code)

    property NavigationControl navigation: NavigationControl{
        accessible.role: MUAccessible.ListItem
        accessible.name: root.name + ". " + root.description
        enabled: root.enabled && root.visible

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.clicked(root.code)
    }

    NavigationFocusBorder {
        navigationCtrl: root.navigation
    }

    RowLayout {
        anchors.fill: parent

        Item {
            width: parent.width / 2.1
            height: parent.height
            clip: true

            Rectangle {
                anchors.fill: parent
                anchors.rightMargin: -12 // NOTE: for a rounded corner on the left side only
                radius: root.radius
                border.width: root.border.width
                border.color: root.border.color

                color: "#595959" // TODO

                StyledTextLabel {
                    anchors.fill: parent

                    text: qsTrc("extensions", "Placeholder")
                    font: ui.theme.titleBoldFont
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Column {
                anchors.fill: parent
                anchors.margins: 58

                spacing: 20

                StyledTextLabel {
                    id: nameLabel

                    width: parent.width

                    font: ui.theme.headerBoldFont
                    horizontalAlignment: Text.AlignLeft
                }

                StyledTextLabel {
                    id: descriptionLabel

                    width: parent.width

                    wrapMode: Text.WordWrap
                    maximumLineCount: 5
                    horizontalAlignment: Text.AlignLeft

                    text: {
                        var breakIndex = description.indexOf('.') // NOTE: first sentence
                        if (breakIndex === -1) {
                            return description
                        }
                        return root.description.substring(0, breakIndex + 1)
                    }
                }
            }
        }
    }

    StyledTextLabel {
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20

        text: qsTrc("extensions", "FREE") // TODO: get from model
        font: ui.theme.tabBoldFont
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.clicked(code)
        }
    }
}
