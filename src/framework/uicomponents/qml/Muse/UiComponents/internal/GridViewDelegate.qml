/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.UiComponents

Item {
    id: root

    property var model: null

    property Component itemDelegate: Item {}
    property string sectionRole: "sectionRole"
    property var sectionValue

    property int cellWidth: 0
    property int cellHeight: 0

    property int rows: 0
    property int rowSpacing: 2
    property int columns: 0
    property int columnSpacing: 2

    width: gridView.columns * gridView.cellWidth - root.columnSpacing
    height: gridView.rows * gridView.cellHeight - root.rowSpacing

    SortFilterProxyModel {
        id: filterModel

        sourceModel: root.model

        filters: [
            FilterValue {
                roleName: root.sectionRole
                roleValue: root.sectionValue
                compareType: CompareType.Equal
            }
        ]
    }

    GridView {
        id: gridView

        readonly property int columns: {
            if (root.columns === -1 && root.rows === -1) {
                return gridView.count
            }

            return root.columns !== -1 ? root.columns : Math.ceil(gridView.count / gridView.rows)
        }

        readonly property int rows: {
            if (root.columns === -1 && root.rows === -1) {
                return 1
            }

            return root.rows !== -1 ? root.rows : Math.ceil(gridView.count / gridView.columns)
        }

        anchors.fill: parent
        anchors.leftMargin: -root.columnSpacing / 2
        anchors.rightMargin: anchors.leftMargin
        anchors.topMargin: -root.rowSpacing / 2
        anchors.bottomMargin: anchors.topMargin

        cellWidth: root.cellWidth + root.columnSpacing
        cellHeight: root.cellHeight + root.rowSpacing

        model: filterModel

        interactive: false
        clip: true

        delegate: Item {
            id: delegateItem

            required property var model

            width: gridView.cellWidth
            height: gridView.cellHeight

            Item {
                anchors.centerIn: parent
                width: root.cellWidth
                height: root.cellHeight

                // Hack: using Repeater to pass `model` as `itemModel` to `root.itemDelegate`
                Repeater {
                    model: [
                        { itemModel: delegateItem.model }
                    ]

                    delegate: root.itemDelegate
                }
            }
        }
    }
}
