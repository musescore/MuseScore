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
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

StyledListView {
    id: root

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    spacing: 0

    signal selectRowRequested(int index)
    signal clearSelectionRequested()

    function positionViewAtSelectedItems() {
        var selectedIndexes = root.model.selectionModel.selectedIndexes
        for (var _index in selectedIndexes) {
            positionViewAtIndex(selectedIndexes[_index].row, ListView.Contain)
        }
    }

    function focusOnFirst() {
        var firstItem = root.itemAtIndex(0)
        if (Boolean(firstItem)) {
            firstItem.navigation.requestActive()
        }
    }

    function clearFocus() {
        root.clearSelectionRequested()
    }

    delegate: ListItemBlank {
        id: itemDelegate

        property var item: model.item

        height: 34
        width: root.width

        isSelected: model.isSelected

        onClicked: {
            root.selectRowRequested(index)
        }

        navigation.name: item.title
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + model.index
        navigation.column: 0
        navigation.accessible.name: item.title
        navigation.onActiveChanged: {
            if (navigation.active) {
                root.positionViewAtIndex(index, ListView.Contain)
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12

            spacing: 4

            VisibilityBox {
                id: visibilityButton

                Layout.alignment: Qt.AlignLeft

                navigation.panel: root.navigationPanel
                navigation.row: itemDelegate.navigation.row
                navigation.column: 1
                accessibleText: titleLabel.text

                isVisible: itemDelegate.item.isVisible

                onVisibleToggled: {
                    itemDelegate.item.isVisible = !itemDelegate.item.isVisible
                }
            }

            StyledTextLabel {
                id: titleLabel

                Layout.fillWidth: true

                horizontalAlignment: Qt.AlignLeft
                text: Boolean(itemDelegate.item) ? itemDelegate.item.title : ""
            }
        }
    }
}
