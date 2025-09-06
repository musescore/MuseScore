/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
import MuseScore.NotationScene

ColumnLayout {
    id: root

    property EmptyStavesVisibilityModel emptyStavesVisibilityModel: null
    property NavigationPanel navigationPanel: null
    property int systemIndex: 0

    spacing: 0

    RowLayout {
        id: headerRow

        Layout.margins: 12

        ColumnLayout {
            spacing: 4

            StyledTextLabel {
                id: title
                Layout.fillWidth: true
                text: qsTrc("notation/staffvisibilitypopup", "Hide empty staves")
                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            StyledTextLabel {
                id: systemNumberLabel
                Layout.fillWidth: true
                text: qsTrc("notation/staffvisibilitypopup", "System %1").arg(root.systemIndex)
                font: ui.theme.bodyFont
                horizontalAlignment: Text.AlignLeft
            }
        }

        FlatButton {
            id: resetAllButton

            enabled: root.emptyStavesVisibilityModel ? root.emptyStavesVisibilityModel.canResetAll : false
            icon: IconCode.UNDO
            toolTipTitle: qsTrc("notation/staffvisibilitypopup", "Reset all")

            navigation.name: "ResetAllButton"
            navigation.panel: root.navigationPanel
            navigation.row: 0

            onClicked: {
                if (root.emptyStavesVisibilityModel) {
                    root.emptyStavesVisibilityModel.resetAllVisibility();
                }
            }
        }
    }

    SeparatorLine {}

    StyledTreeView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        implicitHeight: contentHeight

        model: root.emptyStavesVisibilityModel

        columnWidthProvider: function (column) {
            return width
        }

        delegate: Item {
            id: delegateItem

            // Assigned to by TreeView:
            required property TreeView treeView
            required property bool isTreeNode
            required property bool expanded
            required property bool hasChildren
            required property int depth
            required property int row
            required property int column
            required property bool current

            readonly property var modelIndex: treeView.index(row, column)

            required property var model

            implicitWidth: visibilityControls.implicitWidth
            implicitHeight: 38

            Rectangle {
                anchors.fill: parent
                visible: depth > 0
                color: ui.theme.textFieldColor
            }

            VisibilityControls {
                id: visibilityControls

                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12

                navigationPanel: root.navigationPanel
                navigationRow: delegateItem.row + 1 // +1 for ResetAllButton

                title: delegateItem.model.name
                isRootControl: delegateItem.depth === 0

                useVisibilityButton: true
                isVisible: delegateItem.model.isVisible
                canChangeVisibility: delegateItem.model.canChangeVisibility
                onVisibilityButtonClicked: function (visible) {
                    delegateItem.model.isVisible = visible;
                }

                isExpandable: delegateItem.hasChildren
                isExpanded: delegateItem.expanded
                makeRoomForExpandButton: true
                expandableDepth: delegateItem.depth
                onExpandButtonClicked: function (expand) {
                    expand ? delegateItem.treeView.expand(delegateItem.row) : delegateItem.treeView.collapse(delegateItem.row);
                }
            }
        }
    }
}
