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

DropArea {
    id: root

    required property TreeView treeView
    required property int row
    required property int column
    required property int depth
    required property bool expanded
    required property AbstractLayoutPanelTreeItem item

    required property LayoutPanelTreeModel treeModel
    required property NavigationPanel navigationPanel

    // Shared by all rows, owned by the tree
    required property AbstractLayoutPanelTreeItem draggedItem
    required property bool isGroupCollapsed
    required property PartTreeItem hoveredGroupHeader
    required property PartTreeItem pressedGroupHeader

    property int sideMargin: 0
    property int cardRadius: 0
    property int cardHeight: 30

    readonly property var modelIndex: root.treeView.index(root.row, root.column)
    readonly property PartTreeItem groupHeader: prv.groupPart?.sharedGroupHeader ?? null

    signal groupHoverChanged(bool hovered)
    signal groupPressChanged(bool pressed)

    signal cardPressed(LayoutPanelCard card)
    signal cardReleased()
    signal expandRequested(LayoutPanelCard card, bool expand)
    signal popupOpenedChanged(bool opened)
    signal navigationActivated(string name)
    signal dropRequested()

    QtObject {
        id: prv

        readonly property int itemSpacing: 8
        readonly property int groupPadding: 4
        readonly property int groupRadius: 6

        readonly property bool isDragged: Boolean(root.draggedItem) && root.item === root.draggedItem
        readonly property bool isGroupHeader: root.item?.type === LayoutPanelItemType.SHARED_PART

        readonly property bool isCardBottom: root.depth === 0 ? !root.expanded : Boolean(root.item?.isLastChild)

        // Staves and "Add staff" rows are in their part's group
        readonly property PartTreeItem groupPart: {
            const item = root.depth > 0 ? root.treeModel.modelIndexToItem(root.modelIndex.parent) : root.item
            return item as PartTreeItem
        }

        readonly property int groupRole: prv.groupPart?.sharedGroupRole ?? LayoutPanelItemType.NOT_IN_GROUP

        readonly property bool isInGroup: prv.groupRole !== LayoutPanelItemType.NOT_IN_GROUP
        readonly property bool isInCombinedGroup: !prv.isGroupHeader && Boolean(root.groupHeader?.isEnabled)
        readonly property bool isGroupTop: root.depth === 0 && prv.groupRole === LayoutPanelItemType.GROUP_HEADER
        readonly property bool isCollapsedGroupHeader: prv.isGroupHeader && root.isGroupCollapsed
        readonly property bool isGroupBottom: {
            if (!prv.isCardBottom) {
                return false
            }

            // A collapsed group ends at its header
            return prv.groupRole === LayoutPanelItemType.GROUP_LAST_MEMBER || prv.isCollapsedGroupHeader
        }
    }

    implicitHeight: {
        let height = content.anchors.topMargin + root.cardHeight
        if (!prv.isCardBottom) {
            return height
        }

        if (!prv.isInGroup) {
            return height + prv.itemSpacing
        }

        return height + prv.groupPadding + (prv.isGroupBottom ? prv.itemSpacing : 0)
    }

    // The gaps between cards; clicks on the cards are handled by the cards
    MouseArea {
        anchors.fill: parent
        onClicked: root.treeModel.clearSelection()
    }

    HoverHandler {
        id: groupHover

        // Not over the gap below the group
        readonly property bool isOverGroup: groupHover.hovered && groupHover.point.position.y < groupBackground.height

        enabled: prv.isInGroup

        onIsOverGroupChanged: {
            root.groupHoverChanged(groupHover.isOverGroup)
        }
    }

    // The group's background behind this row
    Rectangle {
        id: groupBackground

        readonly property bool isHovered: Boolean(root.groupHeader) && root.hoveredGroupHeader === root.groupHeader
        readonly property bool isPressed: Boolean(root.groupHeader) && root.pressedGroupHeader === root.groupHeader
        readonly property bool isSelected: Boolean(root.groupHeader?.isSelected)

        anchors.fill: parent
        anchors.bottomMargin: prv.isGroupBottom ? prv.itemSpacing : 0

        visible: prv.isInGroup

        topLeftRadius: prv.isGroupTop ? prv.groupRadius : 0
        topRightRadius: prv.isGroupTop ? prv.groupRadius : 0
        bottomLeftRadius: prv.isGroupBottom ? prv.groupRadius : 0
        bottomRightRadius: prv.isGroupBottom ? prv.groupRadius : 0

        color: groupBackground.isSelected ? ui.theme.accentColor : ui.theme.buttonColor
        opacity: {
            if (groupBackground.isPressed) {
                return 0.7
            }

            if (groupBackground.isHovered) {
                return groupBackground.isSelected ? 0.4 : 0.3
            }

            return 0.5
        }
    }

    Item {
        id: content

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: prv.isGroupTop ? prv.groupPadding : 0
        anchors.leftMargin: prv.isInGroup ? prv.groupPadding : 0
        anchors.rightMargin: prv.isInGroup ? prv.groupPadding : 0

        height: root.cardHeight

        // Where the dragged row will land; its slot follows the drag, as rows are moved live
        Rectangle {
            anchors.fill: parent

            visible: prv.isDragged

            radius: root.cardRadius
            color: Qt.rgba(ui.theme.accentColor.r, ui.theme.accentColor.g, ui.theme.accentColor.b, 0.15)
            border.width: 1.5
            border.color: ui.theme.accentColor
        }

        Loader {
            anchors.fill: parent

            opacity: prv.isDragged ? 0 : 1

            sourceComponent: root.item?.type === LayoutPanelItemType.CONTROL_ADD_STAFF
                             ? addStaffButtonComponent : cardComponent
        }
    }

    Component {
        id: cardComponent

        LayoutPanelCard {
            id: card

            readonly property bool isGroupPressed: prv.isGroupHeader && card.mouseArea.pressed

            onIsGroupPressedChanged: {
                root.groupPressChanged(card.isGroupPressed)
            }

            treeView: root.treeView
            item: root.item
            modelIndex: root.modelIndex
            row: root.row
            depth: root.depth
            isExpanded: prv.isGroupHeader ? !root.isGroupCollapsed : root.expanded
            isInCombinedGroup: prv.isInCombinedGroup

            sideMargin: root.sideMargin
            cardRadius: root.cardRadius

            navigation.name: card.item?.title || "LayoutPanelCard"
            navigation.panel: root.navigationPanel
            navigation.row: root.row
            navigation.onActiveChanged: {
                if (card.navigation.active) {
                    root.navigationActivated(card.navigation.name)
                    root.treeView.positionViewAtRow(root.row, TableView.Contain)
                }
            }

            onClicked: {
                if (card.isSelectable) {
                    root.treeModel.selectRow(root.modelIndex)
                }
            }

            onDoubleClicked: {
                if (card.isExpandable) {
                    root.expandRequested(card, !card.isExpanded)
                }
            }

            onExpandRequested: function(expand) {
                root.expandRequested(card, expand)
            }

            onRemoveSelectionRequested: {
                root.treeModel.removeSelectedRows()
            }

            onChangeVisibilityOfSelectedRowsRequested: function(visible) {
                root.treeModel.changeVisibilityOfSelectedRows(visible)
            }

            onChangeVisibilityRequested: function(modelIndex, visible) {
                root.treeModel.changeVisibility(modelIndex, visible)
            }

            onChangeEnabledOfSelectedRowsRequested: function(enabled) {
                root.treeModel.changeEnabledOfSelectedRows(enabled)
            }

            onChangeEnabledRequested: function(modelIndex, enabled) {
                root.treeModel.changeEnabled(modelIndex, enabled)
            }

            onUngroupRequested: {
                Qt.callLater(root.treeModel.ungroupSharedGroup, card.item.id)
            }

            onMousePressed: {
                root.cardPressed(card)
            }

            onMouseReleased: {
                root.cardReleased()
            }

            onIsPopupOpenedChanged: {
                root.popupOpenedChanged(card.isPopupOpened)
            }
        }
    }

    Component {
        id: addStaffButtonComponent

        LayoutPanelAddStaffButton {
            title: root.item?.title || ""

            navigation.panel: root.navigationPanel
            navigation.row: root.row

            depth: root.depth
            sideMargin: root.sideMargin
            cardRadius: root.cardRadius

            onClicked: {
                root.item.appendNewItem()
            }
        }
    }

    onEntered: function(drag) {
        if (!root.draggedItem || !root.item) {
            return
        }

        if (root.item === root.draggedItem || !root.item.canAcceptDrop(root.draggedItem)) {
            return
        }

        root.dropRequested()
    }
}
