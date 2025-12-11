/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

Item {
    id: root

    property string title: ""
    property int preferredWidth: 0

    property var availableFormats: null

    property real leftMargin: 0
    property real rightMargin: 0

    property int headerCapitalization: Font.AllUppercase

    property alias navigation: navCtrl

    signal clicked()
    signal formatChangeRequested(string formatId)

    implicitHeight: 30
    implicitWidth: Math.max(leftMargin + row.implicitWidth + rightMargin, preferredWidth)

    NavigationControl {
        id: navCtrl
        name: root.objectName !== "" ? root.objectName : "ValueListHeaderItem"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.Button
        accessible.name: {
            var text = root.title + ", "
            if (root.isSorterEnabled) {
                text += root.sortOrder === Qt.AscendingOrder ? qsTrc("ui", "sorted ascending") : qsTrc("ui", "sorted descending")
            } else {
                text += qsTrc("ui", "not sorted")
            }

            return text
        }
        accessible.visualItem: root

        onTriggered: {
            root.clicked()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundSecondaryColor
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        enabled: root.enabled
        hoverEnabled: true

        onClicked: {
            root.clicked()
        }
    }

    RowLayout {
        id: row

        anchors.fill: parent
        anchors.leftMargin: root.leftMargin
        anchors.rightMargin: root.rightMargin

        spacing: 4

        StyledTextLabel {
            id: titleLabel

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            text: title
            horizontalAlignment: Text.AlignLeft
            font.capitalization: root.headerCapitalization
            opacity: ui.theme.buttonOpacityNormal
        }

        MenuButton {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16

            menuModel: root.availableFormats
            visible: menuModel.length > 0

            onHandleMenuItem: function(itemId) {
                root.formatChangeRequested(itemId)
            }
        }
    }

    SeparatorLine {
        anchors.bottom: parent.bottom
        orientation: Qt.Horizontal
    }
    SeparatorLine {
        anchors.right: parent.right
        orientation: Qt.Vertical
    }

    NavigationFocusBorder {
        navigationCtrl: navCtrl
        drawOutsideParent: false
    }

    states: [
        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed

            PropertyChanges {
                target: titleLabel
                opacity: ui.theme.buttonOpacityHover
            }
        }
    ]
}
