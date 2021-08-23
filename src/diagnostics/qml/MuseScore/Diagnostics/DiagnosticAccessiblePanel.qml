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
import MuseScore.Diagnostics 1.0

Rectangle {

    id: root

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        accessibleModel.reload()
    }

    DiagnosticAccessibleModel {
        id: accessibleModel

        property int savedY: 0

        onBeforeReload: {
            accessibleModel.savedY = view.contentY
        }

        onAfterReload: {
            view.contentY = accessibleModel.savedY
        }
    }

    Row {
        id: tools
        anchors.left: parent.left
        anchors.right: parent.right
        height: 48
        spacing: 16

        FlatButton {
            anchors.verticalCenter: parent.verticalCenter
            text: "Refresh"
            onClicked: accessibleModel.reload()
        }

        CheckBox {
            anchors.verticalCenter: parent.verticalCenter
            text: "Auto refresh"
            checked: accessibleModel.isAutoRefresh
            onClicked: accessibleModel.isAutoRefresh = !accessibleModel.isAutoRefresh
        }
    }

    ListView {
        id: view
        anchors.top: tools.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        spacing: 8

        model: accessibleModel
        delegate: Rectangle {
            id: item

            width: parent ? parent.width : 0
            height: 32

            function formatData(data) {
                var str = data.name + "[" + data.role + "]"
                if (data.children > 0) {
                    str += ", children: " + data.children
                }

                str += "\n" + JSON.stringify(data.state)

                return str;
            }

            color: {
                if (itemData.state.focused) {
                    return "#75507b"
                }

                if (itemData.state.active) {
                    return "#73d216"
                }

                return root.color
            }

            StyledTextLabel {
                id: secLabel
                anchors.fill: parent
                anchors.leftMargin: itemData.level * 16
                verticalAlignment: Text.AlignTop
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideNone
                text: item.formatData(itemData)
            }
        }
    }
}
