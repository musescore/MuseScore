/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

import "internal"

Item {
    id: root

    property alias model: tableView.model
    property var sourceComponentCallback

    property var currentEditedCell: null

    property int headerCapitalization: Font.AllUppercase

    property NavigationPanel navigationPanel: NavigationPanel {
        name: root.objectName !== "" ? root.objectName : "TableView"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Both

        accessible.role: MUAccessible.Table
        accessible.name: "Table"
        accessible.visualItem: root
        accessible.enabled: navigationPanel.enabled

        property var navigationIndexForRestore: null
        property bool isNavigationOnHeaders: false

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.AboutActive) {
                if (Boolean(!navigationIndexForRestore)) {
                    return
                }

                var itemForRestore = null

                if (isNavigationOnHeaders) {
                    itemForRestore = horizontalHeader.itemAtCell(navigationIndexForRestore)
                } else {
                    itemForRestore = tableView.itemAtCell(navigationIndexForRestore)
                }

                if (itemForRestore) {
                    event.setData("controlIndex", [itemForRestore.navigation.row, itemForRestore.navigation.column])
                    return
                }
            }
        }
    }

    signal handleItem(var index, var item)

    QtObject {
        id: prv

        property real valueItemWidth: 126
        property real spacing: 4
        property real sideMargin: 30
    }

    Rectangle {
        id: background
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor
        border.width: 1
        border.color: ui.theme.strokeColor
    }

    HorizontalHeaderView {
        id: horizontalHeader

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 1

        syncView: tableView
        resizableColumns: true
        boundsBehavior: tableView.boundsBehavior
        clip: true

        delegate: StyledTableViewColumn {
            leftMargin: index === 0 ? 16 : 8
            rightMargin: index === tableView.model.columnCount() - 1 ? 16 : 8

            title: display.title
            preferredWidth: display.preferredWidth
            availableFormats: display.availableFormats

            headerCapitalization: root.headerCapitalization

            navigation.panel: root.navigationPanel
            navigation.row: 0
            navigation.column: index * 100 // * 100 - some extra space for cell controls

            navigation.onHighlightChanged: {
                if (navigation.highlight) {
                    var cellIndex = Qt.point(index, 0)
                    root.navigationPanel.navigationIndexForRestore = cellIndex
                    root.navigationPanel.isNavigationOnHeaders = true

                    tableView.positionViewAtCell(cellIndex, TableView.Contain)
                }
            }

            onFormatChangeRequested: function(formatId) {
                display.currentFormatId = formatId
            }
        }
    }

    TableView {
        id: tableView

        anchors.left: parent.left
        anchors.top: horizontalHeader.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 1

        clip: true
        rowHeightProvider: (row) => 40
        boundsBehavior: TableView.StopAtBounds
        selectionModel: tableView.model.selectionModel

        ScrollBar.vertical: StyledScrollBar {}
        ScrollBar.horizontal: StyledScrollBar {}

        delegate: StyledTableViewCell {
            property var hHeaderData: tableView.model.headerData(column, Qt.Horizontal)

            cellType: hHeaderData.cellType
            cellEditMode: hHeaderData.cellEditMode
            preferredWidth: hHeaderData.preferredWidth

            sourceComponentCallback: root.sourceComponentCallback

            isSelected: tableView.selectionModel.hasSelection && tableView.selectionModel.isSelected(tableView.model.index(row, column))

            navigation.panel: root.navigationPanel
            navigation.row: row + 1 // + 1 because of the headers
            navigation.column: column * 100 // * 100 - some extra space for cell controls

            navigation.onHighlightChanged: {
                if (navigation.highlight) {
                    var cellIndex = Qt.point(column, row)
                    root.navigationPanel.navigationIndexForRestore = cellIndex
                    root.navigationPanel.isNavigationOnHeaders = false

                    tableView.positionViewAtCell(cellIndex, TableView.Contain)
                }
            }

            onSelectedRequested: function(selected) {
                tableView.selectionModel.select(tableView.model.index(row, column))
            }

            onImplicitWidthChanged: {
                tableView.relayoutIfNeeded()
            }

            Component.onCompleted: {
                tableView.relayoutIfNeeded()
            }

            onEditingStarted: {
                root.currentEditedCell = this
            }

            onEditingFinished: {
                root.currentEditedCell = null
            }
        }

        function relayoutIfNeeded() {
            if (!updateLayout.running) {
                updateLayout.start()
            }
        }

        Timer {
            id: updateLayout
            interval: 10
            repeat: false
            onTriggered: {
                tableView.forceLayout()
            }
        }
    }
}
