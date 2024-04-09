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

    objectName: "DiagnosticAccessiblePanel"

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        accessibleModel.init()
        accessibleModel.reload()
    }

    DiagnosticAccessibleModel {
        id: accessibleModel

        property var lastFocusedIndex: null

        onFocusedItem: function(index) {
            if (lastFocusedIndex) {
                view.collapseBranch(lastFocusedIndex)
            }

            view.expandBranch(index)
            view.positionViewAtIndex(index)
            lastFocusedIndex = index
        }
    }

    Row {
        id: tools
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
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

        function positionViewAtIndex(index) {
            var rows = -1
            while (index.valid) {
                var r = index.row + 1
                rows += r
                index = view.model.parent(index)
            }

            __listView.positionViewAtIndex(rows, ListView.Center)
        }

        function expandBranch(index) {
            var idxs = []
            var parent = view.model.parent(index)
            while (parent.valid) {
                idxs.push(parent)
                parent = view.model.parent(parent)
            }

            for(var i = (idxs.length - 1); i >= 0; --i) {
                var idx = idxs[i]
                view.expand(idx)
            }
        }

        function collapseBranch(index) {
            var idxs = []
            idxs.push(index)
            var parent = view.model.parent(index)
            while (parent.valid) {
                idxs.push(parent)
                parent = view.model.parent(parent)
            }

            for(var i = 0; i < idxs.length; ++i) {
                var idx = idxs[i]
                view.collapse(idx)
            }
        }

        TableViewColumn {
            role: "itemData"
        }

        style: TreeViewStyle {
            rowDelegate: Rectangle {
                height: 48
                width: parent.width
                color: styleData.row%2 == 0 ? root.color : ui.theme.backgroundSecondaryColor
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
                color: ui.theme.accentColor
                visible: styleData.value ? styleData.value.state.focused === 1 : false
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
