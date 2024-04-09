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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Diagnostics 1.0

Rectangle {

    id: root

    objectName: "DiagnosticProfilePanel"
    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        updateContent()
    }

    function updateContent() {
        profModel.reload()
    }

    Item {
        id: toolPanel
        anchors.left: parent.left
        anchors.right: parent.right
        height: 48

        TextInputField {
            id: inputItem
            anchors.left: parent.left
            anchors.right: btnRow.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 16
            clearTextButtonVisible: true
            onTextChanged: function(newTextValue) {
                profModel.find(newTextValue)
            }
        }

        Row {
            id: btnRow
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: 16
            width: childrenRect.width
            spacing: 8

            FlatButton {
                anchors.verticalCenter: parent.verticalCenter
                text: "Update"
                onClicked: root.updateContent()
            }

            FlatButton {
                anchors.verticalCenter: parent.verticalCenter
                text: "Clear"
                onClicked: profModel.clear()
            }

            FlatButton {
                anchors.verticalCenter: parent.verticalCenter
                text: "Print"
                onClicked: profModel.print()
            }
        }
    }

    ProfilerViewModel {
        id: profModel
    }

    ListView {
        anchors.top: toolPanel.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        model: profModel
        section.property: "groupRole"
        section.delegate: Rectangle {
            width: parent.width
            height: 24
            color: ui.theme.backgroundSecondaryColor
            StyledTextLabel {
                anchors.fill: parent
                anchors.margins: 2
                horizontalAlignment: Qt.AlignLeft
                text: section
            }
        }
        delegate: ListItemBlank {

            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            height: 24

            StyledTextLabel {
                anchors.fill: parent
                anchors.leftMargin: 16
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                font.family: "Consolas"
                text: dataRole
            }
        }
    }
}
