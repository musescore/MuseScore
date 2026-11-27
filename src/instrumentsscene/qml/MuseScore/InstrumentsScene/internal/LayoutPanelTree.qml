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

    QtObject {
        id: prv

        property string currentItemNavigationName: ""
        property bool isPopupOpened: false

        readonly property int cardRadius: 3
        readonly property int rowContentHeight: 30
        readonly property int groupPadding: 4
        readonly property int groupRadius: 6

        property LayoutPanelCard pressedDelegate: null
        property real dragStartY: 0

        // Rows clear it only if it's still theirs, as enter/leave can come in either order
        property LayoutPanelTreeDelegate hoveredGroupRow: null
        property PartTreeItem pressedGroupHeader: null

        function startDrag() {
            const delegate = prv.pressedDelegate
            if (!delegate) {
                return
            }

            const isGroupHeader = delegate.type === LayoutPanelItemType.SHARED_PART
            prv.dragStartY = delegate.mapToItem(layoutPanelTreeView.contentItem, 0, 0).y - (isGroupHeader ? prv.groupPadding : 0)

            root.model.startActiveDrag(delegate.modelIndex)
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
                prv.pressedDelegate = null
                root.model.endActiveDrag()
            }
        }
    }

    StyledMenuLoader {
        id: rowContextMenu

        menuAnchorItem: root.Window.window ? root.Window.window.contentItem : null

        onHandleMenuItem: function(itemId) {
            root.model.handleRowContextMenuItem(itemId)
        }
    }

    Connections {
        target: root.model

        // Group instruments are hidden by rowHeightProvider, not by collapse
        function onExpandedChanged(index) {
            const row = layoutPanelTreeView.rowAtIndex(index)
            if (root.model.modelIndexToItem(index).isExpanded) {
                layoutPanelTreeView.expand(row)
            } else {
                layoutPanelTreeView.collapse(row)
            }

            Qt.callLater(layoutPanelTreeView.forceLayout)
        }
    }

    StyledTreeView {
        id: layoutPanelTreeView

        anchors.fill: parent

        interactive: !prv.pressedDelegate && !rowDragHandler.active && !prv.isPopupOpened

        model: root.model

        onLayoutChanged: {
            root.model.restoreNavigation()
        }

        columnWidthProvider: function(column) {
            return layoutPanelTreeView.width
        }

        // 0 hides the row; -1 keeps the delegate's implicitHeight
        rowHeightProvider: function(row) {
            const item = root.model.modelIndexToItem(layoutPanelTreeView.index(row, 0))
            return item?.isHiddenByCollapse() ? 0 : -1
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

            active: Boolean(root.model.draggedItem)

            readonly property bool isGroup: root.model.draggedItem?.type === LayoutPanelItemType.SHARED_PART

            width: layoutPanelTreeView.width
            height: prv.rowContentHeight + (draggedItemLoader.isGroup ? 2 * prv.groupPadding : 0)
            y: prv.dragStartY + rowDragHandler.centroid.position.y - rowDragHandler.centroid.pressPosition.y
            z: 2 // above the rows

            Drag.active: rowDragHandler.active
            Drag.source: draggedItemLoader
            Drag.hotSpot.x: draggedItemLoader.width / 2
            Drag.hotSpot.y: draggedItemLoader.height / 2

            sourceComponent: LayoutPanelCard {
                treeView: layoutPanelTreeView
                item: root.model.draggedItem
                modelIndex: root.model.draggedIndex
                row: -1
                depth: root.model.draggedIndex.parent.valid ? 1 : 0
                isExpanded: false
                isDragged: true

                sideMargin: root.sideMargin
                contentPadding: draggedItemLoader.isGroup ? prv.groupPadding : 0
                cardRadius: draggedItemLoader.isGroup ? prv.groupRadius : prv.cardRadius

                navigation.enabled: false
            }
        }

        delegate: LayoutPanelTreeDelegate {
            id: treeDelegate

            treeModel: root.model
            navigationPanel: treeNavigationPanel

            draggedItem: root.model.draggedItem
            hoveredGroupHeader: prv.hoveredGroupRow?.groupHeader ?? null
            pressedGroupHeader: prv.pressedGroupHeader

            sideMargin: root.sideMargin
            cardRadius: prv.cardRadius
            cardHeight: prv.rowContentHeight
            groupPadding: prv.groupPadding
            groupRadius: prv.groupRadius

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

            // Also sent when the drag takes the grab; the drag clears pressedDelegate when it ends
            onCardReleased: {
                Qt.callLater(function() {
                    if (!rowDragHandler.active) {
                        prv.pressedDelegate = null
                    }
                })
            }

            onExpandRequested: function(card, expand) {
                root.model.setExpanded(card.modelIndex, expand)
            }

            onPopupOpenedChanged: function(opened) {
                prv.isPopupOpened = opened
            }

            onNavigationActivated: function(name) {
                prv.currentItemNavigationName = name
            }

            onDragMoved: function(position) {
                Qt.callLater(root.model.dragOver, treeDelegate.modelIndex, position)
            }

            onContextMenuRequested: function(card, x, y) {
                const pos = card.mapToItem(root, x, y)

                // Once the click is handled
                Qt.callLater(function() {
                    const items = root.model.rowContextMenuItems()
                    if (items.length > 0) {
                        rowContextMenu.open(items, pos.x, pos.y)
                    }
                })
            }
        }
    }
}
