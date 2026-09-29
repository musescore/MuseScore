/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import Muse.UiComponents.LegacyTreeView
import MuseScore.InstrumentsScene

import "internal"

Item {
    id: root

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    property alias contextMenuModel: contextMenuModel

    onVisibleChanged: {
        treeModel.setLayoutPanelVisible(root.visible)
    }

    Rectangle {
        id: background

        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        MouseArea {
            anchors.fill: parent

            onClicked: {
                treeModel.clearSelection()
            }
        }
    }



    LayoutPanelContextMenuModel {
        id: contextMenuModel

        onExpandCollapseAllRequested: function(expand) {
            layoutPanelTreeView.expandCollapseAll(expand)
        }
    }

    Component.onCompleted: {
        contextMenuModel.load()
    }

    QtObject {
        id: prv

        property string currentItemNavigationName: ""

        // Shared-staves groups the user has opened (keyed by shared part id). Groups start collapsed.
        property var expandedGroups: ({})

        readonly property int rowContentHeight: 30
        readonly property int groupPadding: 4
        readonly property int groupMemberSpacing: 2
        readonly property int itemSpacing: 8

        function isGroupExpanded(groupId) {
            // Groups can't be collapsed while their instruments aren't combined
            return !treeModel.isStaveSharingEnabled || Boolean(prv.expandedGroups[groupId])
        }

        function toggleGroupExpanded(groupId) {
            let groups = Object.assign({}, prv.expandedGroups)
            groups[groupId] = !groups[groupId]
            prv.expandedGroups = groups
        }

        // Vertical layout of a tree row. Rows are laid out as "cards": a part and its expanded
        // staves form one card, and consecutive rows of a shared-staves group sit on a common
        // group background. Returns the padding above/below the row content (inside the group
        // background) and the gap below the row (outside it).
        // Interaction state of shared-staves group headers, which is shown on the whole group's background
        // Number of hovered rows per group (rows hand over hover in either order when moving between them)
        property var groupHoverCounts: ({})
        property string pressedGroupId: ""

        function setGroupRowHovered(groupId, hovered) {
            let counts = Object.assign({}, prv.groupHoverCounts)
            counts[groupId] = Math.max(0, (counts[groupId] || 0) + (hovered ? 1 : -1))
            prv.groupHoverCounts = counts
        }
        property var selectedGroups: ({})

        function setGroupSelected(groupId, selected) {
            let groups = Object.assign({}, prv.selectedGroups)
            groups[groupId] = selected
            prv.selectedGroups = groups
        }

        // Bumped whenever rows are added, removed, moved, expanded or collapsed, to re-evaluate row heights
        property int treeRevision: 0

        function rowMetrics(item, index, isExpanded) {
            let m = { hidden: false, top: 0, bottom: 0, gap: 0, height: 0, inGroup: false,
                      groupTop: false, groupBottom: false, cardBottom: true }

            if (!item || !index || !index.valid) {
                return m
            }

            const depth = index.parent.valid ? 1 : 0
            const role = item.sharedGroupRole

            m.inGroup = role !== LayoutPanelItemType.NOT_IN_GROUP

            const isMember = role === LayoutPanelItemType.GROUP_MEMBER || role === LayoutPanelItemType.GROUP_LAST_MEMBER
            if ((isMember && !prv.isGroupExpanded(item.sharedGroupId))
                    || (m.inGroup && item.type === LayoutPanelItemType.CONTROL_ADD_STAFF)) {
                // Instruments in a group can't add staves, so the "Add staff" row is omitted
                m.hidden = true
                return m
            }

            // In groups, the (hidden) "Add staff" control is always the last child
            const visibleSiblingCount = treeModel.rowCount(index.parent) - (depth > 0 && m.inGroup ? 1 : 0)
            const isLastChild = index.row >= visibleSiblingCount - 1

            m.cardBottom = depth === 0 ? !isExpanded : isLastChild

            switch (role) {
            case LayoutPanelItemType.GROUP_HEADER:
                m.top = prv.groupPadding
                m.bottom = prv.groupPadding
                m.groupTop = true
                if (!prv.isGroupExpanded(item.sharedGroupId)) {
                    m.groupBottom = true
                    m.gap = prv.itemSpacing
                }
                break
            case LayoutPanelItemType.GROUP_MEMBER:
                m.bottom = m.cardBottom ? prv.groupMemberSpacing : 0
                break
            case LayoutPanelItemType.GROUP_LAST_MEMBER:
                if (m.cardBottom) {
                    m.bottom = prv.groupPadding
                    m.gap = prv.itemSpacing
                    m.groupBottom = true
                }
                break
            default:
                m.gap = m.cardBottom ? prv.itemSpacing : 0
                break
            }

            m.height = m.top + prv.rowContentHeight + m.bottom + m.gap
            return m
        }
    }

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent

        readonly property int sideMargin: 12
        readonly property int listMargin: 8
        spacing: 0

        LayoutControlPanel {
            id: controlPanel
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            Layout.leftMargin: contentColumn.sideMargin
            Layout.rightMargin: contentColumn.sideMargin

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart

            isMovingUpAvailable: treeModel.isMovingUpAvailable
            isMovingDownAvailable: treeModel.isMovingDownAvailable
            isAddingAvailable: treeModel.isAddingAvailable
            isAddingSystemMarkingsAvailable: treeModel.isAddingSystemMarkingsAvailable
            isRemovingAvailable: treeModel.isRemovingAvailable
            selectedItemsType: treeModel.selectedItemsType

            onAddInstrumentRequested: {
                treeModel.addInstruments()
            }

            onAddSystemMarkingsRequested: {
                treeModel.addSystemMarkings()
            }

            onMoveUpRequested: {
                treeModel.moveSelectedRowsUp()
            }

            onMoveDownRequested: {
                treeModel.moveSelectedRowsDown()
            }

            onRemovingRequested: {
                treeModel.removeSelectedRows()
            }
        }

        ToggleButton {
            Layout.topMargin: 8
            Layout.leftMargin: contentColumn.sideMargin
            Layout.rightMargin: contentColumn.sideMargin

            navigation.panel: controlPanel.navigation
            navigation.order: 5

            text: qsTrc("layoutpanel", "Automatically hide all empty staves")
            checked: treeModel.isHideEmptyStavesEnabled
            onToggled: treeModel.toggleHideEmptyStaves(!checked)
        }

        ToggleButton {
            Layout.topMargin: 8
            Layout.leftMargin: contentColumn.sideMargin
            Layout.rightMargin: contentColumn.sideMargin

            navigation.panel: controlPanel.navigation
            navigation.order: 6

            text: qsTrc("layoutpanel", "Combine instruments onto shared staves")
            checked: treeModel.isStaveSharingEnabled
            onToggled: treeModel.toggleStaveSharing(!checked)
        }

        SeparatorLine {
            Layout.topMargin: contentColumn.sideMargin
        }

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 12
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            text: {
                if (treeModel.addInstrumentsKeyboardShortcut) {
                    //: Keep in sync with the text of the "Add" button at the top of the Layout panel (LayoutControlPanel.qml)
                    return qsTrc("layoutpanel", "There are no instruments in your score. To choose some, press <b>Add</b>, or use the keyboard shortcut %1.")
                    .arg("<b>" + treeModel.addInstrumentsKeyboardShortcut + "</b>")
                } else {
                    //: Keep in sync with the text of the "Add" button at the top of the Layout panel (LayoutControlPanel.qml)
                    return qsTrc("layoutpanel", "There are no instruments in your score. To choose some, press <b>Add</b>.")
                }
            }
            visible: treeModel.isEmpty && treeModel.isAddingAvailable

            verticalAlignment: Qt.AlignTop
            wrapMode: Text.WordWrap
        }

        LegacyTreeView {
            id: layoutPanelTreeView

            // Approximate: rows have variable heights (see prv.rowMetrics)
            readonly property real delegateHeight: 38

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: contentColumn.listMargin
            Layout.leftMargin: contentColumn.listMargin
            Layout.rightMargin: contentColumn.listMargin

            visible: !treeModel.isEmpty

            model: LayoutPanelTreeModel {
                id: treeModel
            }

            selection: treeModel ? treeModel.selectionModel() : null

            onExpanded: prv.treeRevision++
            onCollapsed: prv.treeRevision++

            Connections {
                target: treeModel

                function onRowsInserted() { prv.treeRevision++ }
                function onRowsRemoved() { prv.treeRevision++ }
                function onRowsMoved() { prv.treeRevision++ }
                function onModelReset() { prv.treeRevision++ }
            }

            alternatingRowColors: false
            headerVisible: false

            function expandCollapseAll(expand) {
                for (let row = 0; row < layoutPanelTreeView.model.rowCount(); ++row) {
                    const modelIndex = layoutPanelTreeView.model.index(row, 0);
                    const item = layoutPanelTreeView.model.modelIndexToItem(modelIndex);
                    if (item.isExpandable){
                        if (expand) {
                            layoutPanelTreeView.expand(modelIndex)
                        } else {
                            layoutPanelTreeView.collapse(modelIndex)
                        }
                    }
                }
                flickableItem.returnToBounds();
            }

            function scrollToFocusedItem(focusedIndex) {
                let targetScrollPosition = focusedIndex * layoutPanelTreeView.delegateHeight
                let visibleAreaEnd = flickableItem.contentY + flickableItem.height

                if (targetScrollPosition + layoutPanelTreeView.delegateHeight > visibleAreaEnd) {
                    flickableItem.contentY = Math.min(targetScrollPosition + layoutPanelTreeView.delegateHeight - flickableItem.height, flickableItem.contentHeight - flickableItem.height)
                } else if (targetScrollPosition < flickableItem.contentY) {
                    flickableItem.contentY = Math.max(targetScrollPosition, 0)
                }
            }

            property NavigationPanel navigationTreePanel : NavigationPanel {
                name: "LayoutPanelTree"
                section: root.navigationSection
                direction: NavigationPanel.Both
                enabled: layoutPanelTreeView.enabled && layoutPanelTreeView.visible
                order: controlPanel.navigation.order + 1

                onNavigationEvent: function(event) {
                    if (event.type === NavigationEvent.AboutActive) {
                        event.setData("controlName", prv.currentItemNavigationName)
                    }
                }
            }

            TableViewColumn {
                role: "item"
            }

            function isControl(itemType) {
                return itemType === LayoutPanelItemType.CONTROL_ADD_STAFF
            }

            style: LegacyTreeViewStyle {
                indentation: 0
                branchDelegate: null
                backgroundColor: "transparent"

                // NOTE: the row delegate has no access to the row's model data (only to styleData),
                // so the item is looked up by row. The view's (hidden) row filler has no row at all.
                rowDelegate: Item {
                    id: rowDelegateItem

                    readonly property real rowHeight: {
                        prv.treeRevision
                        if (typeof styleData.row === "undefined" || styleData.row < 0) {
                            return 0
                        }

                        const index = layoutPanelTreeView.__model.mapRowToModelIndex(styleData.row)
                        return prv.rowMetrics(treeModel.modelIndexToItem(index), index, layoutPanelTreeView.isExpanded(index)).height
                    }

                    width: parent.width

                    // The view's Loader resizes this item (which would discard a plain binding), so re-apply on every change
                    Binding {
                        target: rowDelegateItem
                        property: "height"
                        value: rowDelegateItem.rowHeight
                    }
                }
            }

            itemDelegate: DropArea {
                id: dropArea

                readonly property var metrics: { prv.treeRevision; return prv.rowMetrics(rowItem, styleData.index, styleData.isExpanded) }
                readonly property AbstractLayoutPanelTreeItem rowItem: model ? model.item : null
                readonly property AbstractLayoutPanelTreeItem parentRowItem: styleData.depth > 0 ? treeModel.modelIndexToItem(styleData.index.parent) : null

                // Whether this row belongs to a shared-staves group whose instruments are currently combined
                readonly property bool isGroupCombined: {
                    if (!metrics.inGroup || !rowItem) {
                        return false
                    }

                    if (rowItem.type === LayoutPanelItemType.SHARED_PART) {
                        return rowItem.isEnabled
                    }

                    // Origin parts are disabled while their shared part is enabled
                    const partItem = styleData.depth > 0 ? parentRowItem : rowItem
                    return Boolean(partItem) && !partItem.isEnabled
                }

                visible: !metrics.hidden

                // Hovering anywhere on a group (including its instruments) shows the group's hover state
                HoverHandler {
                    id: groupHover
                    enabled: dropArea.metrics.inGroup
                }

                readonly property string groupId: rowItem ? rowItem.sharedGroupId : ""
                readonly property bool isOverGroup: groupHover.hovered && groupHover.point.position.y < height - metrics.gap
                property string hoverReportedGroupId: ""

                function updateGroupHover() {
                    const groupId = isOverGroup ? dropArea.groupId : ""
                    if (groupId === hoverReportedGroupId) {
                        return
                    }
                    if (hoverReportedGroupId !== "") {
                        prv.setGroupRowHovered(hoverReportedGroupId, false)
                    }
                    if (groupId !== "") {
                        prv.setGroupRowHovered(groupId, true)
                    }
                    hoverReportedGroupId = groupId
                }

                onIsOverGroupChanged: updateGroupHover()
                onGroupIdChanged: updateGroupHover()
                Component.onDestruction: {
                    if (hoverReportedGroupId !== "") {
                        prv.setGroupRowHovered(hoverReportedGroupId, false)
                    }
                }

                // Shared-staves group background: one segment per row, rounded at the group's ends
                Item {
                    width: parent.width
                    height: parent.height - dropArea.metrics.gap

                    visible: dropArea.metrics.inGroup
                    clip: true

                    Rectangle {
                        readonly property int overflow: 8

                        readonly property string groupId: dropArea.rowItem ? dropArea.rowItem.sharedGroupId : ""
                        readonly property bool isSelected: Boolean(prv.selectedGroups[groupId])
                        readonly property bool isHovered: (prv.groupHoverCounts[groupId] || 0) > 0
                        readonly property bool isPressed: prv.pressedGroupId === groupId
                        readonly property bool isFilled: dropArea.isGroupCombined || isSelected || isHovered || isPressed

                        width: parent.width
                        y: dropArea.metrics.groupTop ? 0 : -overflow
                        height: parent.height + (dropArea.metrics.groupTop ? 0 : overflow) + (dropArea.metrics.groupBottom ? 0 : overflow)

                        radius: 6
                        color: isSelected ? ui.theme.accentColor : isFilled ? ui.theme.buttonColor : "transparent"
                        border.width: isFilled ? 0 : 1
                        border.color: ui.theme.strokeColor

                        opacity: {
                            if (isSelected) {
                                return isPressed ? 0.7 : isHovered ? 0.4 : 0.5
                            }
                            if (dropArea.isGroupCombined) {
                                return isPressed ? 0.7 : isHovered ? 0.3 : 0.5
                            }
                            // Outline only (instruments not combined)
                            return isPressed ? 0.5 : isHovered ? 0.3 : 1
                        }
                    }
                }

                Loader {
                    id: treeItemDelegateLoader

                    property int delegateType: model ? model.item.type : LayoutPanelItemType.UNDEFINED

                    height: parent.height
                    width: parent.width

                    sourceComponent: layoutPanelTreeView.isControl(delegateType) ?
                                         controlItemDelegateComponent : treeItemDelegateComponent

                    Component {
                        id: treeItemDelegateComponent

                        LayoutPanelItemDelegate {
                            id: itemDelegate

                            treeView: layoutPanelTreeView
                            item: model?.item ?? null
                            modelIndex: styleData.index
                            depth: styleData.depth
                            isExpanded: styleData.isExpanded

                            contentTopMargin: dropArea.metrics.top
                            contentHeight: prv.rowContentHeight
                            cardBottom: dropArea.metrics.cardBottom
                            isInGroup: dropArea.metrics.inGroup
                            isGroupCombined: dropArea.isGroupCombined
                            isGroupExpanded: Boolean(item) && prv.isGroupExpanded(item.sharedGroupId)
                            isStaveSharingEnabled: treeModel.isStaveSharingEnabled
                            isLastRow: styleData.depth === 0 && !styleData.hasSibling

                            navigation.name: item?.title || "LayoutPanelItemDelegate"
                            navigation.panel: layoutPanelTreeView.navigationTreePanel
                            navigation.row: model?.index ?? 0
                            navigation.onActiveChanged: {
                                if (navigation.active) {
                                    prv.currentItemNavigationName = navigation.name
                                    layoutPanelTreeView.scrollToFocusedItem(model.index)
                                }
                            }

                            onClicked: {
                                if (itemDelegate.isSelectable) {
                                    treeModel.selectRow(styleData.index)
                                }
                            }

                            readonly property bool isGroupHeader: type === LayoutPanelItemType.SHARED_PART
                            readonly property string groupId: item ? item.sharedGroupId : ""

                            readonly property bool isHeaderPressed: isGroupHeader && mouseArea.pressed
                            onIsHeaderPressedChanged: {
                                prv.pressedGroupId = isHeaderPressed ? groupId : (prv.pressedGroupId === groupId ? "" : prv.pressedGroupId)
                            }

                            readonly property bool isHeaderSelected: isGroupHeader && isSelected
                            onIsHeaderSelectedChanged: prv.setGroupSelected(groupId, isHeaderSelected)
                            Component.onCompleted: {
                                if (isHeaderSelected) {
                                    prv.setGroupSelected(groupId, true)
                                }
                            }

                            onGroupExpandToggled: {
                                prv.toggleGroupExpanded(item.sharedGroupId)
                            }

                            onDoubleClicked: {
                                if (type === LayoutPanelItemType.SHARED_PART) {
                                    if (treeModel.isStaveSharingEnabled) {
                                        prv.toggleGroupExpanded(item.sharedGroupId)
                                    }
                                    return
                                }

                                if (!isExpandable) {
                                    return
                                }

                                if (!styleData.isExpanded) {
                                    layoutPanelTreeView.expand(styleData.index)
                                } else {
                                    layoutPanelTreeView.collapse(styleData.index)
                                }
                            }

                            onRemoveSelectionRequested: {
                                treeModel.removeSelectedRows()
                            }

                            onChangeVisibilityOfSelectedRowsRequested: function(visible) {
                                treeModel.changeVisibilityOfSelectedRows(visible);
                            }

                            onChangeVisibilityRequested: function(modelIndex, visible) {
                                treeModel.changeVisibility(modelIndex, visible)
                            }

                            onChangeEnabledOfSelectedRowsRequested: function(enabled) {
                                treeModel.changeEnabledOfSelectedRows(enabled);
                            }

                            onChangeEnabledRequested: function(modelIndex, enabled) {
                                treeModel.changeEnabled(modelIndex, enabled)
                            }

                            onDragStarted: {
                                treeModel.startActiveDrag()
                            }

                            onDropped: {
                                treeModel.endActiveDrag()
                            }

                            onIsPopupOpenedChanged: {
                                layoutPanelTreeView.flickableItem.interactive = !itemDelegate.isPopupOpened
                            }
                        }
                    }

                    Component {
                        id: controlItemDelegateComponent

                        LayoutPanelItemControl {
                            title: model?.item?.title || ""

                            navigation.panel: layoutPanelTreeView.navigationTreePanel
                            navigation.row: model?.index || 0

                            contentHeight: prv.rowContentHeight

                            onClicked: {
                                styleData.value.appendNewItem()
                            }
                        }
                    }
                }

                onEntered: function(drag) {
                    const draggedItem = drag.source as LayoutPanelItemDelegate
                    if (!draggedItem) {
                        return
                    }

                    if (styleData.index === draggedItem.modelIndex || !styleData.value.canAcceptDrop(draggedItem.item)) {
                        return
                    }

                    if (draggedItem.modelIndex.row < 0 || styleData.index.row < 0) {
                        return;
                    }

                    Qt.callLater(treeModel.moveRows,
                                 draggedItem.modelIndex.parent,
                                 draggedItem.modelIndex.row,
                                 1,
                                 styleData.index.parent,
                                 styleData.index.row)
                }
            }
        }
    }
}
