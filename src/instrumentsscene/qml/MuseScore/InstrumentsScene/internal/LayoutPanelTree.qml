/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

import Muse.Ui
import Muse.UiComponents

import MuseScore.InstrumentsScene

Item {
    id: root

    required property LayoutPanelTreeModel model

    property NavigationSection navigationSection: null
    property int navigationOrder: 0

    property int sideMargin: 0

    function expandCollapseAll(expand) {
        for (let i = 0; i < root.model.rowCount(); ++i) {
            const modelIndex = root.model.index(i, 0)
            if (!root.model.modelIndexToItem(modelIndex).isExpandable) {
                continue
            }

            const row = layoutPanelTreeView.rowAtIndex(modelIndex)
            if (expand) {
                layoutPanelTreeView.expand(row)
            } else {
                layoutPanelTreeView.collapse(row)
            }
        }
    }

    QtObject {
        id: prv

        property string currentItemNavigationName: ""
        property bool isPopupOpened: false

        property LayoutPanelItemDelegate pressedDelegate: null

        property AbstractLayoutPanelTreeItem draggedItem: null
        property var draggedIndex: null
        property int draggedDepth: 0
        property real dragStartY: 0

        property int focusedRow: -1
        property int focusedColumn: -1
        property bool focusedHighlight: false

        function setExpanded(delegate, expand) {
            if (treeNavigationPanel.active) {
                const navigation = prv.findNavigation(delegate, function(nav) { return nav.active })
                if (navigation) {
                    prv.focusedRow = delegate.row
                    prv.focusedColumn = navigation.column
                    prv.focusedHighlight = navigation.highlight
                }
            }

            if (expand) {
                layoutPanelTreeView.expand(delegate.row)
            } else {
                layoutPanelTreeView.collapse(delegate.row)
            }
        }

        //! NOTE Expanding or collapsing doesn't move the toggled row
        function restoreFocus() {
            const row = prv.focusedRow
            prv.focusedRow = -1
            if (row < 0) {
                return
            }

            const cell = layoutPanelTreeView.itemAtCell(Qt.point(0, row))
            if (!cell) {
                return
            }

            const column = prv.focusedColumn
            const navigation = prv.findNavigation(cell, function(nav) { return nav.column === column })
            if (navigation) {
                navigation.requestActive(prv.focusedHighlight)
            }
        }

        //! NOTE Walks the row's items, so that VisibilityControls' buttons are found too
        function findNavigation(item, predicate) {
            const navigation = item.navigation
            if (navigation && navigation.panel === treeNavigationPanel && predicate(navigation)) {
                return navigation
            }

            for (let i = 0; i < item.children.length; ++i) {
                const found = prv.findNavigation(item.children[i], predicate)
                if (found) {
                    return found
                }
            }

            return null
        }

        function releasePressedDelegate() {
            if (!rowDragHandler.active) {
                prv.pressedDelegate = null
            }
        }

        function startDrag() {
            const delegate = prv.pressedDelegate
            if (!delegate) {
                return
            }

            prv.draggedIndex = delegate.modelIndex
            prv.draggedDepth = delegate.depth
            prv.dragStartY = delegate.mapToItem(layoutPanelTreeView.contentItem, 0, 0).y
            prv.draggedItem = delegate.item

            root.model.startActiveDrag()

            if (delegate.isExpanded) {
                layoutPanelTreeView.collapse(delegate.row)
            }
        }

        function endDrag() {
            const wasDragging = Boolean(prv.draggedItem)

            prv.draggedItem = null
            prv.draggedIndex = null
            prv.pressedDelegate = null

            if (wasDragging) {
                root.model.endActiveDrag()
            }
        }

        function moveDraggedItem(targetIndex) {
            const sourceIndex = prv.draggedIndex
            if (!sourceIndex || sourceIndex.row < 0 || targetIndex.row < 0) {
                return
            }

            if (root.model.moveRows(sourceIndex.parent, sourceIndex.row, 1, targetIndex.parent, targetIndex.row)) {
                prv.draggedIndex = root.model.index(targetIndex.row, 0, targetIndex.parent)
            }
        }
    }

    NavigationPanel {
        id: treeNavigationPanel

        name: "LayoutPanelTree"
        section: root.navigationSection
        direction: NavigationPanel.Both
        enabled: root.enabled && root.visible
        order: root.navigationOrder

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.AboutActive) {
                event.setData("controlName", prv.currentItemNavigationName)
            }
        }
    }

    //! NOTE Outside the rows, so the drag survives row rebuilds
    DragHandler {
        id: rowDragHandler

        target: null
        xAxis.enabled: false
        yAxis.enabled: Boolean(prv.pressedDelegate)

        onActiveChanged: {
            if (rowDragHandler.active) {
                prv.startDrag()
            } else {
                prv.endDrag()
            }
        }
    }

    StyledTreeView {
        id: layoutPanelTreeView

        anchors.fill: parent

        interactive: !prv.pressedDelegate && !rowDragHandler.active && !prv.isPopupOpened

        model: root.model

        onLayoutChanged: {
            prv.restoreFocus()
        }

        columnWidthProvider: function(column) {
            return layoutPanelTreeView.width
        }

        Loader {
            id: draggedItemLoader

            active: Boolean(prv.draggedItem)

            width: layoutPanelTreeView.width
            height: 38
            y: prv.dragStartY + rowDragHandler.centroid.position.y - rowDragHandler.centroid.pressPosition.y
            z: 2 // above the rows

            Drag.active: rowDragHandler.active
            Drag.source: draggedItemLoader
            Drag.hotSpot.x: draggedItemLoader.width / 2
            Drag.hotSpot.y: draggedItemLoader.height / 2

            sourceComponent: LayoutPanelItemDelegate {
                treeView: layoutPanelTreeView
                item: prv.draggedItem
                modelIndex: prv.draggedIndex
                row: -1
                depth: prv.draggedDepth
                isExpanded: false
                isDragged: true

                sideMargin: root.sideMargin

                navigation.enabled: false
            }
        }

        delegate: DropArea {
            id: dropArea

            required property TreeView treeView
            required property int row
            required property int column
            required property int depth
            required property bool expanded
            required property AbstractLayoutPanelTreeItem item

            readonly property var modelIndex: dropArea.treeView.index(dropArea.row, dropArea.column)

            implicitHeight: 38

            Loader {
                anchors.fill: parent

                opacity: Boolean(prv.draggedItem) && dropArea.item === prv.draggedItem ? 0 : 1

                sourceComponent: dropArea.item?.type === LayoutPanelItemType.CONTROL_ADD_STAFF
                                 ? controlItemDelegateComponent : treeItemDelegateComponent

                Component {
                    id: treeItemDelegateComponent

                    LayoutPanelItemDelegate {
                        id: itemDelegate

                        treeView: layoutPanelTreeView
                        item: dropArea.item
                        modelIndex: dropArea.modelIndex
                        row: dropArea.row
                        depth: dropArea.depth
                        isExpanded: dropArea.expanded

                        sideMargin: root.sideMargin

                        navigation.name: itemDelegate.item?.title || "LayoutPanelItemDelegate"
                        navigation.panel: treeNavigationPanel
                        navigation.row: dropArea.row
                        navigation.onActiveChanged: {
                            if (itemDelegate.navigation.active) {
                                prv.currentItemNavigationName = itemDelegate.navigation.name
                                layoutPanelTreeView.positionViewAtRow(dropArea.row, TableView.Contain)
                            }
                        }

                        onClicked: {
                            if (itemDelegate.isSelectable) {
                                root.model.selectRow(dropArea.modelIndex)
                            }
                        }

                        onDoubleClicked: {
                            if (itemDelegate.isExpandable) {
                                prv.setExpanded(itemDelegate, !itemDelegate.isExpanded)
                            }
                        }

                        onExpandRequested: function(expand) {
                            prv.setExpanded(itemDelegate, expand)
                        }

                        onRemoveSelectionRequested: {
                            root.model.removeSelectedRows()
                        }

                        onChangeVisibilityOfSelectedRowsRequested: function(visible) {
                            root.model.changeVisibilityOfSelectedRows(visible)
                        }

                        onChangeVisibilityRequested: function(modelIndex, visible) {
                            root.model.changeVisibility(modelIndex, visible)
                        }

                        onChangeEnabledOfSelectedRowsRequested: function(enabled) {
                            root.model.changeEnabledOfSelectedRows(enabled)
                        }

                        onChangeEnabledRequested: function(modelIndex, enabled) {
                            root.model.changeEnabled(modelIndex, enabled)
                        }

                        onMousePressed: {
                            prv.pressedDelegate = itemDelegate
                        }

                        onMouseReleased: {
                            Qt.callLater(prv.releasePressedDelegate)
                        }

                        onIsPopupOpenedChanged: {
                            prv.isPopupOpened = itemDelegate.isPopupOpened
                        }
                    }
                }

                Component {
                    id: controlItemDelegateComponent

                    LayoutPanelItemControl {
                        title: dropArea.item?.title || ""
                        isSelected: dropArea.item?.isSelected || false

                        navigation.panel: treeNavigationPanel
                        navigation.row: dropArea.row

                        depth: dropArea.depth
                        sideMargin: root.sideMargin

                        onClicked: {
                            dropArea.item.appendNewItem()
                        }
                    }
                }
            }

            onEntered: function(drag) {
                if (!prv.draggedItem || !dropArea.item) {
                    return
                }

                if (dropArea.item === prv.draggedItem || !dropArea.item.canAcceptDrop(prv.draggedItem)) {
                    return
                }

                Qt.callLater(prv.moveDraggedItem, dropArea.modelIndex)
            }
        }
    }
}
