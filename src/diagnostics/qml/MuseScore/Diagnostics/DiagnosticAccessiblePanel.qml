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
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Diagnostics 1.0

Rectangle {

    id: root

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        accessibleModel.init()
        accessibleModel.reload()
    }

    DiagnosticAccessibleModel {
        id: accessibleModel

        property int savedY: 0

        onBeforeReload: {
            //accessibleModel.savedY = view.contentY
        }

        onAfterReload: {
            //view.contentY = accessibleModel.savedY
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

    TreeView {
        id: view
        anchors.top: tools.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true

        headerVisible: false

        model: accessibleModel

        TableViewColumn {
            role: "itemData"
        }


        style: TreeViewStyle {
            indentation: styleData.depth

//            frame: Item {}
//            incrementControl: Item {}
//            decrementControl: Item {}
//            handle: Item {}
//            scrollBarBackground: Item {}
//            branchDelegate: Item {}

           // backgroundColor: root.color

            rowDelegate: Rectangle {
                height: 48
                width: parent.width
                color: root.color
            }
        }

        itemDelegate: Item {
            id: item

            function formatData(data) {
                var str = data.name + " [" + data.role + "]"
                if (data.children > 0) {
                    str += ", children: " + data.children
                }

                str += "\n" + JSON.stringify(data.state)

                return str;
            }

            Rectangle {
                anchors.fill: parent
                color: ui.theme.backgroundSecondaryColor
                visible: styleData.row%2 == 1
            }

            StyledTextLabel {
                id: secLabel
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideNone
                text: item.formatData(styleData.value)
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (!styleData.isExpanded) {
                        view.expand(styleData.index)
                    } else {
                        view.collapse(styleData.index)
                    }
                }
            }
        }
    }
}
