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

Item {
    id: root

    property alias model: treeView.model

    Rectangle {
        id: background
        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor
    }

    TreeView {
        id: treeView

        anchors.fill: parent
        anchors.topMargin: 12

        alternatingRowColors: false
        headerVisible: false
        frameVisible: false

        TableViewColumn {
            role: "itemRole"
        }

        style: TreeViewStyle {
            indentation: 0

            frame: Item {}
            incrementControl: Item {}
            decrementControl: Item {}
            handle: Item {}
            scrollBarBackground: Item {}
            branchDelegate: Item {}

            backgroundColor: background.color

            rowDelegate: Rectangle {
                id: rowTreeDelegate

                height: 36
                width: parent.width
                color: background.color
            }
        }

        itemDelegate: GradientTabButton {
            property bool expanded: Boolean(model) ? model.itemRole.expanded : false

            orientation: Qt.Horizontal

            spacing: 16
            leftPadding: spacing * (styleData.depth + 1)

            normalStateFont: ui.theme.bodyFont
            selectedStateFont: ui.theme.bodyBoldFont

            title: Boolean(model) ? model.itemRole.title : ""
            checked: Boolean(model) && Boolean(model.itemRole) ? model.itemRole.id === treeView.model.currentPageId : false

            Component.onCompleted: {
                updateExpandedState()
            }

            onExpandedChanged: {
                updateExpandedState()
            }

            function updateExpandedState() {
                if (expanded) {
                    treeView.expand(styleData.index)
                } else {
                    treeView.collapse(styleData.index)
                }
            }

            iconComponent: StyledIconLabel {
                width: 24
                height: width
                iconCode: Boolean(model) ? model.itemRole.icon : IconCode.NONE
            }

            onClicked: {
                treeView.model.selectRow(styleData.index)
            }
        }
    }
}
