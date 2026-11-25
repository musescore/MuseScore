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

    readonly property real contentHeight: layoutPanelTreeView.contentHeight

    function expandCollapseAll(expand) {
        let groups = Object.assign({}, prv.expandedGroups)

        for (let i = 0; i < root.model.rowCount(); ++i) {
            const modelIndex = root.model.index(i, 0)
            const item = root.model.modelIndexToItem(modelIndex)
            if (!item.isExpandable) {
                continue
            }

            if (item.type === LayoutPanelItemType.SHARED_PART) {
                groups[item.id] = expand
                continue
            }

            const row = layoutPanelTreeView.rowAtIndex(modelIndex)
            if (expand) {
                layoutPanelTreeView.expand(row)
            } else {
                layoutPanelTreeView.collapse(row)
            }
        }

        prv.expandedGroups = groups
        layoutPanelTreeView.forceLayout()
    }

    QtObject {
        id: prv

        property string currentItemNavigationName: ""
        property bool isPopupOpened: false

        readonly property int cardRadius: 3
        readonly property int rowContentHeight: 30

        property LayoutPanelCard pressedDelegate: null

        property AbstractLayoutPanelTreeItem draggedItem: null
        property var draggedIndex: null
        property int draggedDepth: 0
        property real dragStartY: 0

        property int focusedRow: -1
        property int focusedColumn: -1
        property bool focusedHighlight: false

        property var expandedGroups: ({})

        // Rows clear it only if it's still theirs, as enter/leave can come in either order
        property LayoutPanelTreeDelegate hoveredGroupRow: null
        property PartTreeItem pressedGroupHeader: null

        //! NOTE Only combined groups can be collapsed
        function isGroupCollapsed(part) {
            const header = part?.sharedGroupHeader ?? null
            return Boolean(header) && header.isEnabled && !prv.expandedGroups[header.id]
        }

        function setExpanded(delegate, expand) {
            if (treeNavigationPanel.active) {
                const navigation = prv.findNavigation(delegate, function(nav) { return nav.active })
                if (navigation) {
                    prv.focusedRow = delegate.row
                    prv.focusedColumn = navigation.column
                    prv.focusedHighlight = navigation.highlight
                }
            }

            if (delegate.type === LayoutPanelItemType.SHARED_PART) {
                let groups = Object.assign({}, prv.expandedGroups)
                groups[delegate.item.id] = expand
                prv.expandedGroups = groups

                layoutPanelTreeView.forceLayout()
                return
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

        function cancelDrag() {
            prv.draggedItem = null
            prv.draggedIndex = null
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

    Connections {
        target: root.model

        function onModelAboutToBeReset() {
            prv.cancelDrag()
        }

        //! NOTE Combining a group on or off changes no rows, so the view isn't relaid out by itself
        function onSharedGroupEnabledChanged() {
            Qt.callLater(layoutPanelTreeView.forceLayout)
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

        // 0 hides a collapsed group's instruments and their staves; -1 keeps the delegate's implicitHeight
        rowHeightProvider: function(row) {
            const index = layoutPanelTreeView.index(row, 0)
            const part = root.model.modelIndexToItem(index.parent.valid ? index.parent : index) as PartTreeItem
            const isMember = Boolean(part) && part.type === LayoutPanelItemType.PART

            return isMember && prv.isGroupCollapsed(part) ? 0 : -1
        }

        // Blank space below the last row; sits behind the rows
        MouseArea {
            parent: layoutPanelTreeView
            anchors.fill: parent
            z: -0.5

            onClicked: root.model.clearSelection()
        }

        Loader {
            id: draggedItemLoader

            active: Boolean(prv.draggedItem)

            width: layoutPanelTreeView.width
            height: prv.rowContentHeight
            y: prv.dragStartY + rowDragHandler.centroid.position.y - rowDragHandler.centroid.pressPosition.y
            z: 2 // above the rows

            Drag.active: rowDragHandler.active
            Drag.source: draggedItemLoader
            Drag.hotSpot.x: draggedItemLoader.width / 2
            Drag.hotSpot.y: draggedItemLoader.height / 2

            sourceComponent: LayoutPanelCard {
                treeView: layoutPanelTreeView
                item: prv.draggedItem
                modelIndex: prv.draggedIndex
                row: -1
                depth: prv.draggedDepth
                isExpanded: false
                isDragged: true

                sideMargin: root.sideMargin
                cardRadius: prv.cardRadius

                navigation.enabled: false
            }
        }

        delegate: LayoutPanelTreeDelegate {
            id: treeDelegate

            treeModel: root.model
            navigationPanel: treeNavigationPanel

            draggedItem: prv.draggedItem
            isGroupCollapsed: prv.isGroupCollapsed(treeDelegate.item as PartTreeItem)
            hoveredGroupHeader: prv.hoveredGroupRow?.groupHeader ?? null
            pressedGroupHeader: prv.pressedGroupHeader

            sideMargin: root.sideMargin
            cardRadius: prv.cardRadius
            cardHeight: prv.rowContentHeight

            onGroupHoverChanged: function(hovered) {
                if (hovered) {
                    prv.hoveredGroupRow = treeDelegate
                } else if (prv.hoveredGroupRow === treeDelegate) {
                    prv.hoveredGroupRow = null
                }
            }

            onGroupPressChanged: function(pressed) {
                prv.pressedGroupHeader = pressed ? treeDelegate.groupHeader : null
            }

            onCardPressed: function(card) {
                prv.pressedDelegate = card
            }

            onCardReleased: {
                Qt.callLater(prv.releasePressedDelegate)
            }

            onExpandRequested: function(card, expand) {
                prv.setExpanded(card, expand)
            }

            onPopupOpenedChanged: function(opened) {
                prv.isPopupOpened = opened
            }

            onNavigationActivated: function(name) {
                prv.currentItemNavigationName = name
            }

            onDropRequested: {
                Qt.callLater(prv.moveDraggedItem, treeDelegate.modelIndex)
            }
        }
    }
}
