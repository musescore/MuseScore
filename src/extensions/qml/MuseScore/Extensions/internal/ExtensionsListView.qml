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
import MuseScore.Extensions 1.0

Column {
    id: root

    property alias title: titleLabel.text

    property alias model: filterModel.sourceModel
    readonly property alias count: view.count

    property alias filters: filterModel.filters

    property string selectedExtensionCode: ""

    property var flickableItem: null
    property int headerHeight: titleLabel.height + spacing

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "ExtensionsListView"
        direction: NavigationPanel.Both
        accessible.name: root.title
    }

    function focusOnFirst() {
        view.itemAtIndex(0).requestActive()
    }

    signal clicked(int index, var extension, var navigationControl)
    signal navigationActivated(var itemRect)

    SortFilterProxyModel {
        id: filterModel
    }

    StyledTextLabel {
        id: titleLabel
        width: parent.width
        font: ui.theme.tabBoldFont
        horizontalAlignment: Text.AlignLeft
    }

    spacing: 24

    GridView {
        id: view

        readonly property int columns: Math.max(0, Math.floor(width / cellWidth))

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: -spacingBetweenColumns / 2
        anchors.rightMargin: -spacingBetweenColumns / 2

        height: contentHeight - spacingBetweenRows

        model: filterModel

        // We don't want this GridView to be scrollable; it would conflict with the containing Flickable.
        interactive: false
        highlightFollowsCurrentItem: false // prevent automatic scrolling

        readonly property real spacingBetweenColumns: 40
        readonly property real spacingBetweenRows: 24

        readonly property real actualCellWidth: 656
        readonly property real actualCellHeight: 224

        cellWidth: actualCellWidth + spacingBetweenColumns
        cellHeight: actualCellHeight + spacingBetweenRows

        delegate: Item {
            id: item

            height: view.cellHeight
            width: view.cellWidth

            function requestActive() {
                _item.navigation.requestActive()
            }

            ExtensionItem {
                id: _item

                width: view.actualCellWidth
                height: view.actualCellHeight

                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter

                code: model.code
                name: model.name
                description: model.description
                status: model.status

                selected: selectedExtensionCode === model.code

                navigation.panel: root.navigationPanel
                navigation.row: view.columns === 0 ? 0 : Math.floor(model.index / view.columns)
                navigation.column: model.index - (navigation.row * view.columns)
                navigation.onActiveChanged: {
                    var pos = mapToItem(root.flickableItem, 0, 0)
                    var rect = Qt.rect(pos.x, pos.y, _item.width, _item.height)
                    root.navigationActivated(rect)
                }

                onClicked: {
                    forceActiveFocus()
                    root.clicked(index, model, navigation)
                }
            }
        }
    }
}
