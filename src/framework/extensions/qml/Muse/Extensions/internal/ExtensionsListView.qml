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
import Muse.Extensions 1.0

Column {
    id: root

    property alias model: filterModel.sourceModel
    readonly property alias count: view.count

    property alias title: titleLabel.text

    property string search: ""
    property string selectedCategory: ""
    property bool pluginIsEnabled: false
    property string selectedPluginUri: ""

    property var flickableItem: null
    property int headerHeight: titleLabel.height + spacing

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "PluginsListView"
        direction: NavigationPanel.Both
        accessible.name: root.title
    }

    signal pluginClicked(var plugin, var navigationControl)
    signal navigationActivated(var itemRect)

    function focusOnFirst() {
        // Force the view to load the items. Without this, `view.itemAtIndex(0)` might be null even when `count > 0`,
        // causing navigation to break when calling this function from `resetNavigationFocus()` in `PluginsPage`.
        view.forceLayout()

        view.itemAtIndex(0).requestActive()
    }

    SortFilterProxyModel {
        id: filterModel

        filters: [
            FilterValue {
                roleName: "name"
                roleValue: root.search
                compareType: CompareType.Contains
            },
            FilterValue {
                roleName: "enabled"
                roleValue: root.pluginIsEnabled
                compareType: CompareType.Equal
            },
            FilterValue {
                roleName: "category"
                roleValue: root.selectedCategory
                compareType: CompareType.Contains
            }
        ]
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

        readonly property real spacingBetweenColumns: 50
        readonly property real spacingBetweenRows: 24

        readonly property real actualCellWidth: 256
        readonly property real actualCellHeight: 176

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

                name: model.name
                thumbnailUrl: model.thumbnailUrl
                selected: model.uri === root.selectedPluginUri

                navigation.panel: root.navigationPanel
                navigation.row: view.columns === 0 ? 0 : Math.floor(model.index / view.columns)
                navigation.column: model.index - (navigation.row * view.columns)
                navigation.onActiveChanged: {
                    var pos = _item.mapToItem(root.flickableItem, 0, 0)
                    var rect = Qt.rect(pos.x, pos.y, _item.width, _item.height)
                    root.navigationActivated(rect)
                }

                onClicked: {
                    forceActiveFocus()
                    root.pluginClicked(model, navigation)
                }
            }
        }
    }
}
