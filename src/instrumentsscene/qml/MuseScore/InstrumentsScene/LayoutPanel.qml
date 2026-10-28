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

            // Only below the header: clicks on its (disabled) buttons and switches don't deselect
            onClicked: function(mouse) {
                if (mouse.y > headerDivider.mapToItem(background, 0, 0).y) {
                    treeModel.clearSelection()
                }
            }
        }
    }




    // Right-click menu for the selected instruments / system markings
    StyledMenuLoader {
        id: rowContextMenu

        menuAnchorItem: root.Window.window ? root.Window.window.contentItem : null

        onHandleMenuItem: function(itemId) {
            prv.handleRowContextMenuItem(itemId)
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
        readonly property int groupMemberSpacing: 4
        readonly property int itemSpacing: 8

        // A group being dragged (by its header) is shown collapsed for the duration of the drag
        property string draggedGroupId: ""

        // While an instrument is dragged over a group it can join, that group is the drop target: { id, isEmpty }
        property var dropGroupTarget: null
        property bool edgeMovePending: false
        // The group the dragged instrument was in when the drag started (if any)
        property string dragStartGroupId: ""

        // Selected instruments (ids, in list order) and system markings, for the right-click menu
        property var contextMenuPartIds: []

        function openRowContextMenu(x, y) {
            let partIds = []
            let systemMarkingsCount = 0
            let groups = []
            // The group all selected instruments are in, if they're all in the same one (it isn't offered)
            let currentGroupId = undefined

            for (let row = 0; row < treeModel.rowCount(); ++row) {
                const item = treeModel.modelIndexToItem(treeModel.index(row, 0))
                if (!item) {
                    continue
                }
                if (item.isSelected && item.type === LayoutPanelItemType.PART) {
                    partIds.push(item.id)
                    const groupId = prv.isGroupMember(item) ? item.sharedGroupId : ""
                    currentGroupId = currentGroupId === undefined || currentGroupId === groupId ? groupId : ""
                } else if (item.isSelected && item.type === LayoutPanelItemType.SYSTEM_OBJECTS_LAYER) {
                    ++systemMarkingsCount
                }
                if (item.type === LayoutPanelItemType.SHARED_PART || item.type === LayoutPanelItemType.EMPTY_SHARED_GROUP) {
                    groups.push(item)
                }
            }

            if (partIds.length === 0 && systemMarkingsCount === 0) {
                return
            }

            prv.contextMenuPartIds = partIds

            const several = partIds.length > 1
            let items = []

            if (partIds.length > 0 && systemMarkingsCount === 0) {
                if (treeModel.isMasterScore) {
                    items.push({ id: "duplicate", title: several ? qsTrc("layoutpanel", "Duplicate instruments")
                                                                 : qsTrc("layoutpanel", "Duplicate instrument") })
                }
                items.push({ id: "remove", title: several ? qsTrc("layoutpanel", "Remove instruments")
                                                          : qsTrc("layoutpanel", "Remove instrument") })
            } else if (partIds.length === 0) {
                items.push({ id: "remove", title: qsTrc("layoutpanel", "Remove system markings") })
            } else {
                items.push({ id: "remove", title: qsTrc("layoutpanel", "Remove selected") })
            }

            // System markings in a mixed selection simply aren't moved
            if (partIds.length > 0 && groups.length > 0 && treeModel.isMasterScore) {
                let groupItems = []
                for (const group of groups) {
                    if (group.id === currentGroupId) {
                        continue
                    }
                    const isEmpty = group.type === LayoutPanelItemType.EMPTY_SHARED_GROUP
                    groupItems.push({ id: "move-to-group:" + (isEmpty ? "empty:" : "shared:") + group.id, title: group.title })
                }
                groupItems.push({})
                groupItems.push({ id: "move-to-new-group", title: several ? qsTrc("layoutpanel", "New group with instruments")
                                                                         : qsTrc("layoutpanel", "New group with instrument") })

                items.push({ id: "move-to-group", title: qsTrc("layoutpanel", "Move to shared stave group"), subitems: groupItems })
            }

            rowContextMenu.open(items, x, y)
        }

        function handleRowContextMenuItem(itemId) {
            const partIds = prv.contextMenuPartIds
            prv.contextMenuPartIds = []

            if (itemId === "duplicate") {
                treeModel.duplicateParts(partIds)
            } else if (itemId === "remove") {
                treeModel.removeSelectedRows()
            } else if (itemId === "move-to-new-group") {
                const sharedPartId = treeModel.createSharedGroupWithParts(partIds)
                if (sharedPartId !== "") {
                    prv.expandGroup(sharedPartId)
                }
            } else if (itemId.startsWith("move-to-group:")) {
                const parts = itemId.split(":")
                const sharedPartId = treeModel.movePartsToSharedGroup(partIds, parts[2], parts[1] === "empty")
                if (sharedPartId !== "") {
                    prv.expandGroup(sharedPartId)
                }
            }
        }

        function expandGroup(groupId) {
            let expanded = Object.assign({}, prv.expandedGroups)
            expanded[groupId] = true
            prv.expandedGroups = expanded
        }

        function isDropGroupTarget(groupId) {
            return Boolean(prv.dropGroupTarget) && prv.dropGroupTarget.id === groupId
        }

        // isCombined: whether the group's instruments are currently combined (its toggle is on, and so is "Combine")
        function isGroupExpanded(groupId, isCombined) {
            if (groupId !== "" && groupId === prv.draggedGroupId) {
                return false
            }

            // Groups can't be collapsed while their instruments aren't combined
            return !isCombined || Boolean(prv.expandedGroups[groupId])
        }

        // Whether a row's group is combined: the header is enabled exactly while it is, and its instruments
        // are disabled exactly while it is (staves follow their instrument)
        function isRowGroupCombined(item, parentItem) {
            if (!item) {
                return false
            }
            if (item.type === LayoutPanelItemType.SHARED_PART) {
                return item.isEnabled
            }
            const partItem = parentItem ? parentItem : item
            return !partItem.isEnabled
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

        // In parts, instruments of the main score that aren't in the part are only listed on request;
        // that resets whenever another score or part is shown (but not when the same part reloads)
        property bool showsOtherInstruments: false
        property string otherInstrumentsScoreName: ""
        readonly property bool hidesOtherInstruments: !treeModel.isMasterScore && !prv.showsOtherInstruments

        readonly property bool hasOtherInstruments: {
            prv.treeRevision
            if (treeModel.isMasterScore) {
                return false
            }
            for (let row = 0; row < treeModel.rowCount(); ++row) {
                const item = treeModel.modelIndexToItem(treeModel.index(row, 0))
                if (item && item.type === LayoutPanelItemType.PART && !item.existsInScore) {
                    return true
                }
            }
            return false
        }

        function showOtherInstruments() {
            prv.showsOtherInstruments = true
        }

        function isGroupMember(item) {
            return Boolean(item) && (item.sharedGroupRole === LayoutPanelItemType.GROUP_MEMBER
                                     || item.sharedGroupRole === LayoutPanelItemType.GROUP_LAST_MEMBER)
        }

        // Number of instrument rows following a group header (top level rows of the same group)
        function groupMemberCount(headerIndex) {
            const header = treeModel.modelIndexToItem(headerIndex)
            let count = 0
            for (let row = headerIndex.row + 1; row < treeModel.rowCount(headerIndex.parent); ++row) {
                const item = treeModel.modelIndexToItem(treeModel.index(row, 0, headerIndex.parent))
                if (!prv.isGroupMember(item) || item.sharedGroupId !== header.sharedGroupId) {
                    break
                }
                ++count
            }
            return count
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

            // Empty group: its header, then a "Drag instruments here" row, on one group background
            if (item.type === LayoutPanelItemType.EMPTY_SHARED_GROUP) {
                m.inGroup = true
                m.top = prv.groupPadding
                m.bottom = prv.groupPadding + prv.rowContentHeight + prv.groupPadding
                m.groupTop = true
                m.groupBottom = true
                m.gap = prv.itemSpacing
                m.height = m.top + prv.rowContentHeight + m.bottom + m.gap
                return m
            }

            if (prv.hidesOtherInstruments && depth === 0 && item.type === LayoutPanelItemType.PART && !item.existsInScore) {
                m.hidden = true
                return m
            }

            // Staves can't be added to instruments that aren't in the part
            const parentItem = depth > 0 ? treeModel.modelIndexToItem(index.parent) : null
            const parentInScore = !parentItem || parentItem.existsInScore
            if (item.type === LayoutPanelItemType.CONTROL_ADD_STAFF && !parentInScore) {
                m.hidden = true
                return m
            }

            m.inGroup = role !== LayoutPanelItemType.NOT_IN_GROUP

            const isMember = role === LayoutPanelItemType.GROUP_MEMBER || role === LayoutPanelItemType.GROUP_LAST_MEMBER
            if ((isMember && !prv.isGroupExpanded(item.sharedGroupId, prv.isRowGroupCombined(item, parentItem)))
                    || (m.inGroup && item.type === LayoutPanelItemType.CONTROL_ADD_STAFF)) {
                // Instruments in a group can't add staves, so the "Add staff" row is omitted
                m.hidden = true
                return m
            }

            // In groups, the (hidden) "Add staff" control is always the last child
            const visibleSiblingCount = treeModel.rowCount(index.parent) - (depth > 0 && (m.inGroup || !parentInScore) ? 1 : 0)
            const isLastChild = index.row >= visibleSiblingCount - 1

            m.cardBottom = depth === 0 ? !isExpanded : isLastChild

            switch (role) {
            case LayoutPanelItemType.GROUP_HEADER:
                m.top = prv.groupPadding
                m.bottom = prv.groupPadding
                m.groupTop = true
                if (!prv.isGroupExpanded(item.sharedGroupId, item.isEnabled)) {
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

        // No top margin: the dock already leaves 12px below its tabs, matching the space above the divider
        StyledTextLabel {
            Layout.fillWidth: true
            Layout.leftMargin: contentColumn.sideMargin
            Layout.rightMargin: contentColumn.sideMargin

            text: treeModel.isMasterScore
                  ? qsTrc("layoutpanel", "Main score")
                  //: %1 is the name of a part
                  : qsTrc("layoutpanel", "Part: %1").arg(treeModel.scoreName)
            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        LayoutControlPanel {
            id: controlPanel
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 8

            Layout.leftMargin: contentColumn.sideMargin
            Layout.rightMargin: contentColumn.sideMargin

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart

            isMovingUpAvailable: treeModel.isMovingUpAvailable
            isMovingDownAvailable: treeModel.isMovingDownAvailable
            isAddingAvailable: treeModel.isAddingAvailable
            isAddingSystemMarkingsAvailable: treeModel.isAddingSystemMarkingsAvailable
            // Shared staves only exist in the main score
            isAddingSharedStaveGroupAvailable: treeModel.isAddingAvailable && treeModel.isMasterScore
            isRemovingAvailable: treeModel.isRemovingAvailable
            selectedItemsType: treeModel.selectedItemsType

            onAddInstrumentRequested: {
                treeModel.addInstruments()
            }

            onAddSystemMarkingsRequested: {
                treeModel.addSystemMarkings()
            }

            onAddSharedStaveGroupRequested: {
                treeModel.addEmptySharedGroup()
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

            // Shared staves aren't shown in parts
            visible: treeModel.isMasterScore

            text: qsTrc("layoutpanel", "Combine instruments onto shared staves")
            checked: treeModel.isStaveSharingEnabled
            onToggled: treeModel.toggleStaveSharing(!checked)
        }

        SeparatorLine {
            id: headerDivider
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
            // While other instruments are hidden (in parts), the list is only as tall as its content,
            // so that the "Show other instruments" button sits right below it
            Layout.fillHeight: !prv.hidesOtherInstruments
            Layout.preferredHeight: prv.hidesOtherInstruments ? flickableItem.contentHeight : -1
            Layout.maximumHeight: prv.hidesOtherInstruments ? flickableItem.contentHeight : Number.POSITIVE_INFINITY
            Layout.topMargin: contentColumn.listMargin
            Layout.leftMargin: contentColumn.listMargin
            Layout.rightMargin: contentColumn.listMargin

            visible: !treeModel.isEmpty

            model: LayoutPanelTreeModel {
                id: treeModel

                onScoreChanged: {
                    if (treeModel.scoreName !== prv.otherInstrumentsScoreName) {
                        prv.otherInstrumentsScoreName = treeModel.scoreName
                        prv.showsOtherInstruments = false
                    }
                }
            }

            selection: treeModel ? treeModel.selectionModel() : null

            onExpanded: prv.treeRevision++
            onCollapsed: prv.treeRevision++

            // Clicking blank space below the last row deselects everything. Sits in the view behind its rows
            // (above the view's own mouse area); the rows' gaps are handled by each row (see the item delegate)
            MouseArea {
                parent: layoutPanelTreeView.flickableItem
                anchors.fill: parent
                z: -0.5

                onClicked: treeModel.clearSelection()
            }

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

                    if (rowItem.type === LayoutPanelItemType.EMPTY_SHARED_GROUP) {
                        return treeModel.isStaveSharingEnabled
                    }

                    // Origin parts are disabled while their shared part is enabled
                    const partItem = styleData.depth > 0 ? parentRowItem : rowItem
                    return Boolean(partItem) && !partItem.isEnabled
                }

                visible: !metrics.hidden

                // Clicking the row's blank space (its spacing / group padding around the content) deselects everything;
                // clicks on the content itself are handled by the row
                MouseArea {
                    anchors.fill: parent
                    onClicked: treeModel.clearSelection()
                }

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

                readonly property LayoutPanelItemDelegate rowDelegate: treeItemDelegateLoader.item as LayoutPanelItemDelegate
                // A group being dragged (by its header): its slot shows a group-sized drop placeholder instead
                readonly property bool isDraggedGroup: Boolean(rowDelegate) && rowDelegate.isDragged && Boolean(rowItem)
                                                       && (rowItem.type === LayoutPanelItemType.SHARED_PART
                                                           || rowItem.type === LayoutPanelItemType.EMPTY_SHARED_GROUP)

                // Shared-staves group background: one segment per row, rounded at the group's ends
                Item {
                    width: parent.width
                    height: parent.height - dropArea.metrics.gap

                    visible: dropArea.metrics.inGroup && !dropArea.isDraggedGroup
                    clip: true

                    Rectangle {
                        readonly property int overflow: 8

                        readonly property string groupId: dropArea.rowItem ? dropArea.rowItem.sharedGroupId : ""
                        readonly property bool isSelected: Boolean(prv.selectedGroups[groupId])
                        readonly property bool isHovered: (prv.groupHoverCounts[groupId] || 0) > 0
                        readonly property bool isPressed: prv.pressedGroupId === groupId
                        // Versions > Group style: "Filled groups" fills every group; "Filled only when on" fills combined groups
                        // and outlines uncombined ones; "Outlined groups" outlines every group (see the separate outline below).
                        // In every style, hover, press and selection fill the group.
                        readonly property bool outlined: contextMenuModel.groupStyle === 2
                        readonly property bool filledWhenOff: contextMenuModel.groupStyle === 0
                        // Whether the group is filled at rest (hover, press and selection always fill it)
                        readonly property bool hasRestingFill: !outlined && (dropArea.isGroupCombined || filledWhenOff)
                        readonly property bool isFilled: hasRestingFill || isSelected || isHovered || isPressed

                        width: parent.width
                        y: dropArea.metrics.groupTop ? 0 : -overflow
                        height: parent.height + (dropArea.metrics.groupTop ? 0 : overflow) + (dropArea.metrics.groupBottom ? 0 : overflow)

                        radius: 6
                        color: isSelected ? ui.theme.accentColor : isFilled ? ui.theme.buttonColor : "transparent"
                        border.width: !outlined && !isFilled ? 1 : 0 // "Filled only when on": outline of an uncombined group
                        border.color: ui.theme.strokeColor

                        opacity: {
                            if (!isFilled) {
                                return outlined ? 0 : 1
                            }
                            if (isSelected) {
                                return isPressed ? 0.7 : isHovered ? 0.4 : 0.5
                            }
                            if (hasRestingFill) {
                                return isPressed ? 0.7 : isHovered ? 0.3 : 0.5
                            }
                            return isPressed ? 0.5 : 0.3
                        }
                    }

                    // High contrast: group stroke (the theme's border width is only non-zero in high contrast themes)
                    Rectangle {
                        readonly property int overflow: 8

                        visible: ui.theme.borderWidth > 0

                        width: parent.width
                        y: dropArea.metrics.groupTop ? 0 : -overflow
                        height: parent.height + (dropArea.metrics.groupTop ? 0 : overflow) + (dropArea.metrics.groupBottom ? 0 : overflow)

                        radius: 6
                        color: "transparent"
                        border.width: ui.theme.borderWidth
                        border.color: ui.theme.strokeColor
                    }

                    // Group outline, in every state ("Outlined groups" only)
                    Rectangle {
                        readonly property int overflow: 8

                        visible: contextMenuModel.groupStyle === 2

                        width: parent.width
                        y: dropArea.metrics.groupTop ? 0 : -overflow
                        height: parent.height + (dropArea.metrics.groupTop ? 0 : overflow) + (dropArea.metrics.groupBottom ? 0 : overflow)

                        radius: 6
                        color: "transparent"
                        border.width: 1
                        border.color: Qt.rgba(ui.theme.fontPrimaryColor.r, ui.theme.fontPrimaryColor.g, ui.theme.fontPrimaryColor.b, 0.3)
                    }
                }

                // Drop target highlight: while an instrument is dragged over a group it can be added to
                Item {
                    width: parent.width
                    height: parent.height - dropArea.metrics.gap

                    visible: dropArea.metrics.inGroup && Boolean(dropArea.rowItem) && prv.isDropGroupTarget(dropArea.rowItem.sharedGroupId)
                    clip: true
                    z: 1

                    Rectangle {
                        readonly property int overflow: 8

                        width: parent.width
                        y: dropArea.metrics.groupTop ? 0 : -overflow
                        height: parent.height + (dropArea.metrics.groupTop ? 0 : overflow) + (dropArea.metrics.groupBottom ? 0 : overflow)

                        radius: 6
                        color: "transparent"
                        border.width: 1.5
                        border.color: ui.theme.accentColor
                    }
                }

                // Empty group: the row instruments can be dropped on
                Rectangle {
                    x: 4
                    y: dropArea.metrics.top + prv.rowContentHeight + prv.groupPadding
                    width: parent.width - 8
                    height: prv.rowContentHeight

                    visible: Boolean(dropArea.rowItem) && dropArea.rowItem.type === LayoutPanelItemType.EMPTY_SHARED_GROUP
                             && !dropArea.isDraggedGroup

                    radius: 3
                    color: ui.theme.textFieldColor

                    StyledTextLabel {
                        anchors.centerIn: parent
                        text: qsTrc("layoutpanel", "Drag instruments here")
                    }
                }

                // Drop placeholder: while a row is dragged, its slot in the list (which follows the
                // drag, as rows are moved live) shows where it will be dropped
                // For a dragged group, it's the size of the (collapsed) group itself, and only outlined
                Rectangle {
                    readonly property bool isGroup: dropArea.isDraggedGroup

                    x: isGroup ? 0 : 4
                    y: isGroup ? 0 : dropArea.metrics.top
                    width: parent.width - (isGroup ? 0 : 8)
                    height: isGroup ? parent.height - dropArea.metrics.gap : prv.rowContentHeight

                    // Not while dropping onto a group (which is highlighted instead)
                    visible: Boolean(dropArea.rowDelegate) && dropArea.rowDelegate.isDragged && !prv.dropGroupTarget

                    radius: isGroup ? 6 : 3
                    color: isGroup ? "transparent" : Qt.rgba(ui.theme.accentColor.r, ui.theme.accentColor.g, ui.theme.accentColor.b, 0.15)
                    border.width: 1.5
                    border.color: ui.theme.accentColor
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
                            // Empty groups are always shown open
                            isGroupExpanded: Boolean(item) && (type === LayoutPanelItemType.EMPTY_SHARED_GROUP
                                                               || prv.isGroupExpanded(item.sharedGroupId, dropArea.isGroupCombined))
                            isStaveSharingEnabled: treeModel.isStaveSharingEnabled
                            // Re-evaluated on every row change (the view's "has sibling" flag isn't updated when rows move)
                            isLastRow: {
                                prv.treeRevision
                                return styleData.depth === 0 && styleData.index.row === treeModel.rowCount() - 1
                            }
                            isParentVisible: !dropArea.parentRowItem || dropArea.parentRowItem.isVisible
                            isLocked: type === LayoutPanelItemType.SYSTEM_OBJECTS_LAYER && styleData.depth === 0 && styleData.index.row === 0

                            groupHeight: dropArea.height - dropArea.metrics.gap
                            hideDisabledEyes: contextMenuModel.hideDisabledEyes
                            onlyShowSettingsOnHover: contextMenuModel.onlyShowSettingsOnHover

                            // Versions > Indentation: in combined groups, instruments' (and their staves') contents are indented.
                            // Full: their eyes line up with the group's expand button (30px eye + 2px gap);
                            // Half: their expand button's left edge lines up with the middle of the group's expand button
                            contentIndent: {
                                if (!dropArea.isGroupCombined || type === LayoutPanelItemType.SHARED_PART
                                        || type === LayoutPanelItemType.EMPTY_SHARED_GROUP) {
                                    return 0
                                }
                                switch (contextMenuModel.groupIndentation) {
                                case 1: return prv.rowContentHeight + 2
                                case 2: return prv.rowContentHeight / 2
                                default: return 0
                                }
                            }

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
                                                                  || type === LayoutPanelItemType.EMPTY_SHARED_GROUP
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

                            onContextMenuRequested: function(x, y) {
                                if (!isSelected) {
                                    treeModel.selectRow(styleData.index)
                                }
                                const pos = itemDelegate.mapToItem(root, x, y)
                                Qt.callLater(prv.openRowContextMenu, pos.x, pos.y)
                            }

                            onUngroupRequested: {
                                Qt.callLater(treeModel.ungroupSharedGroup, item.id)
                            }

                            onDeleteEmptyGroupRequested: {
                                Qt.callLater(treeModel.deleteEmptySharedGroup, item.id)
                            }

                            onGroupExpandToggled: {
                                prv.toggleGroupExpanded(item.sharedGroupId)
                            }

                            onDoubleClicked: {
                                if (type === LayoutPanelItemType.SHARED_PART) {
                                    if (isGroupCombined) {
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
                                prv.dropGroupTarget = null
                                prv.dragStartGroupId = prv.isGroupMember(item) ? item.sharedGroupId : ""
                                if (isGroupHeader) {
                                    prv.draggedGroupId = groupId
                                }
                                treeModel.startActiveDrag()
                            }

                            onDropped: {
                                if (isGroupHeader && prv.draggedGroupId === groupId) {
                                    prv.draggedGroupId = ""
                                }

                                const dropTarget = prv.dropGroupTarget
                                const startGroupId = prv.dragStartGroupId
                                prv.dropGroupTarget = null
                                prv.dragStartGroupId = ""

                                const isPart = type === LayoutPanelItemType.PART
                                const partId = isPart ? item.id : ""
                                const leftGroup = isPart && !dropTarget && startGroupId !== ""
                                        && !(prv.isGroupMember(item) && item.sharedGroupId === startGroupId)

                                // Joining or leaving a group includes moving the instrument, so that it's one undoable step
                                if (isPart && dropTarget) {
                                    treeModel.cancelActiveDrag()
                                } else if (leftGroup) {
                                    treeModel.endActiveDragLeavingSharedGroup(partId)
                                    return
                                } else {
                                    treeModel.endActiveDrag()
                                }

                                if (dropTarget) {
                                    // Dropped on a group: join it
                                    Qt.callLater(function() {
                                        if (dropTarget.isEmpty) {
                                            const sharedPartId = treeModel.fillEmptySharedGroup(dropTarget.id, partId)
                                            if (sharedPartId !== "") {
                                                prv.expandGroup(sharedPartId)
                                            }
                                        } else {
                                            treeModel.addPartToSharedGroup(partId, dropTarget.id)
                                        }
                                    })
                                }
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

                onEntered: function(drag) { dropArea.handleDrag(drag, false) }
                // Within a row, only for the group edge zones (see below)
                onPositionChanged: function(drag) { dropArea.handleDrag(drag, true) }

                function handleDrag(drag, isPositionChange) {
                    const draggedItem = drag.source as LayoutPanelItemDelegate
                    if (!draggedItem) {
                        return
                    }

                    const dragged = draggedItem.item
                    const target = styleData.value
                    if (!dragged || !target) {
                        return
                    }

                    const draggedGroupId = prv.isGroupMember(dragged) ? dragged.sharedGroupId : ""
                    const targetGroupId = target.type === LayoutPanelItemType.SHARED_PART || prv.isGroupMember(target)
                            ? target.sharedGroupId : ""

                    // Instruments can join another group (or an empty one): while dragged over it, it is the drop
                    // target (the rows don't move), and the instrument joins it on release. Near the group's top or
                    // bottom edge, the instrument goes just before / after the group instead (e.g. between two groups)
                    const isEmptyTarget = target.type === LayoutPanelItemType.EMPTY_SHARED_GROUP
                    const joinsGroup = dragged.type === LayoutPanelItemType.PART
                            && (isEmptyTarget || (targetGroupId !== "" && targetGroupId !== draggedGroupId))

                    if (isPositionChange && !joinsGroup) {
                        return
                    }

                    if (joinsGroup) {
                        const edgeZone = 12
                        // Rows are compared at the top level: a staff row (of the group's last, expanded instrument)
                        // stands for its instrument
                        const topLevelIndex = styleData.depth > 0 ? styleData.index.parent : styleData.index
                        const row = topLevelIndex.row
                        const isHeaderRow = isEmptyTarget || target.type === LayoutPanelItemType.SHARED_PART
                        const groupLastRow = target.type === LayoutPanelItemType.SHARED_PART ? row + prv.groupMemberCount(topLevelIndex) : row

                        let besideRow = -1 // the row to place the instrument directly before / after
                        let after = false
                        if (isHeaderRow && drag.y < edgeZone) {
                            besideRow = row
                        } else if (dropArea.metrics.groupBottom && drag.y > dropArea.height - dropArea.metrics.gap - edgeZone) {
                            besideRow = groupLastRow
                            after = true
                        }

                        if (besideRow < 0) {
                            prv.dropGroupTarget = { id: isEmptyTarget ? target.id : targetGroupId, isEmpty: isEmptyTarget }
                            return
                        }

                        prv.dropGroupTarget = null

                        const sourceRow = draggedItem.modelIndex.row
                        const moveToRow = after ? (sourceRow < besideRow ? besideRow : besideRow + 1)
                                                : (sourceRow < besideRow ? besideRow - 1 : besideRow)
                        if (moveToRow !== sourceRow && sourceRow >= 0 && !prv.edgeMovePending) {
                            // Position changes come in quickly: don't queue the same move twice
                            prv.edgeMovePending = true
                            const sourceParent = draggedItem.modelIndex.parent
                            const destinationParent = topLevelIndex.parent
                            Qt.callLater(function() {
                                prv.edgeMovePending = false
                                treeModel.moveRows(sourceParent, sourceRow, 1, destinationParent, moveToRow)
                            })
                        }
                        return
                    }
                    prv.dropGroupTarget = null

                    if (styleData.index === draggedItem.modelIndex || !styleData.value.canAcceptDrop(draggedItem.item)) {
                        return
                    }

                    // Nothing else (whole groups, system markings, other instruments' staves) can be moved in among
                    // a group's instruments; instruments can be reordered within their group, or moved out of it
                    // (leaving it on release)
                    if (prv.isGroupMember(target) && targetGroupId !== draggedGroupId) {
                        return
                    }

                    const sourceRow = draggedItem.modelIndex.row
                    let destinationRow = styleData.index.row
                    if (sourceRow < 0 || destinationRow < 0) {
                        return;
                    }

                    // A group header moves together with its instruments
                    const count = dragged.type === LayoutPanelItemType.SHARED_PART ? 1 + prv.groupMemberCount(draggedItem.modelIndex) : 1

                    // Moving down onto a group header places the row(s) after the whole group (not between the
                    // header and its instruments); moving up onto it places them before the group
                    if (target.type === LayoutPanelItemType.SHARED_PART && destinationRow > sourceRow) {
                        destinationRow += prv.groupMemberCount(styleData.index)
                    }

                    if (destinationRow >= sourceRow && destinationRow < sourceRow + count) {
                        return
                    }

                    Qt.callLater(treeModel.moveRows,
                                 draggedItem.modelIndex.parent,
                                 sourceRow,
                                 count,
                                 styleData.index.parent,
                                 destinationRow)
                }
            }
        }

        FlatButton {
            Layout.fillWidth: true
            Layout.preferredHeight: prv.rowContentHeight
            Layout.leftMargin: contentColumn.listMargin
            Layout.rightMargin: contentColumn.listMargin

            visible: prv.hidesOtherInstruments && prv.hasOtherInstruments && !treeModel.isEmpty

            navigation.panel: layoutPanelTreeView.navigationTreePanel
            navigation.row: treeModel.rowCount() + 1
            navigation.column: 0

            text: qsTrc("layoutpanel", "Show other instruments")

            onClicked: prv.showOtherInstruments()
        }

        Item {
            Layout.fillHeight: true
            visible: prv.hidesOtherInstruments
        }
    }
}
