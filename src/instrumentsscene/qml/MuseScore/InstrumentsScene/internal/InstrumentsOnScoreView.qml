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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.InstrumentsScene

Item {
    id: root

    required property InstrumentsOnScoreListModel instrumentsOnScoreModel

    property alias navigation: instrumentsView.navigation

    // Shows instruments that aren't in the score yet in italics (not wanted when creating a new score)
    property bool highlightsNewInstruments: false

    // Adds the instruments where they belong in the list, keeping the list where it was scrolled to, unless the new
    // instrument would be out of view: then the list scrolls only as far as needed to show it
    function addInstruments(instrumentIdList) {
        const contentY = instrumentsView.contentY
        const row = root.instrumentsOnScoreModel.addInstruments(instrumentIdList)

        instrumentsView.contentY = contentY
        instrumentsView.forceLayout()

        if (row >= 0) {
            instrumentsView.positionViewAtIndex(row, ListView.Contain)
        }
    }

    QtObject {
        id: prv

        readonly property int rowContentHeight: 30
        readonly property int groupPadding: 4
        readonly property int groupMemberSpacing: 4
        readonly property int itemSpacing: 8

        property var expandedGroups: ({})
        property var groupHoverCounts: ({})
        property var selectedGroups: ({})
        property string pressedGroupId: ""

        function changeGroupHover(groupId, delta) {
            const counts = Object.assign({}, groupHoverCounts)
            counts[groupId] = Math.max(0, (counts[groupId] || 0) + delta)
            groupHoverCounts = counts
        }

        function setGroupExpanded(groupId, expanded) {
            const groups = Object.assign({}, expandedGroups)
            groups[groupId] = expanded
            expandedGroups = groups
        }

        function setGroupSelected(groupId, selected) {
            if (Boolean(selectedGroups[groupId]) === selected) {
                return
            }
            const groups = Object.assign({}, selectedGroups)
            groups[groupId] = selected
            selectedGroups = groups
        }
    }

    StyledTextLabel {
        id: instrumentsLabel

        anchors.top: parent.top
        anchors.left: parent.left

        font: ui.theme.bodyBoldFont
        text: qsTrc("instruments", "Your score")
    }

    RowLayout {
        id: operationsRow

        anchors.top: instrumentsLabel.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right

        StyledDropdown {
            id: ordersDropdown

            Layout.fillWidth: true

            navigation.name: "Orders"
            navigation.panel: instrumentsView.navigation
            navigation.row: 0
            navigation.column: 0

            model: root.instrumentsOnScoreModel.orders

            currentIndex: root.instrumentsOnScoreModel.currentOrderIndex

            displayText: qsTrc("instruments", "Order:") + " " + currentText

            onActivated: function(index, value) {
                root.instrumentsOnScoreModel.currentOrderIndex = index
            }
        }

        // Make soloist: on while all selected instruments are soloists; in a mixed selection, makes them all soloists
        FlatButton {
            Layout.preferredWidth: width

            navigation.name: "MakeSoloist"
            navigation.panel: instrumentsView.navigation
            navigation.row: 0
            navigation.column: 1
            navigation.accessible.name: toolTipTitle

            icon: IconCode.ACCOUNT
            accentButton: root.instrumentsOnScoreModel.isSelectionSoloist
            toolTipTitle: root.instrumentsOnScoreModel.isSelectionSoloist ? qsTrc("instruments", "Remove soloist")
                                                                          : qsTrc("instruments", "Make soloist")

            enabled: root.instrumentsOnScoreModel.isSoloistAvailable

            onClicked: {
                root.instrumentsOnScoreModel.toggleSoloistForSelection()
            }
        }

        FlatButton {
            Layout.preferredWidth: width

            navigation.name: "Delete"
            navigation.panel: instrumentsView.navigation
            navigation.row: 0
            navigation.column: 2

            icon: IconCode.DELETE_TANK
            toolTipTitle: qsTrc("instruments", "Remove selected instruments from score")

            enabled: root.instrumentsOnScoreModel.isRemovingAvailable

            onClicked: {
                root.instrumentsOnScoreModel.removeSelection()
            }
        }
    }

    StyledListView {
        id: instrumentsView

        anchors.top: operationsRow.bottom
        anchors.topMargin: 8
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        model: root.instrumentsOnScoreModel

        accessible.name: instrumentsLabel.text

        delegate: ListItemBlank {
            id: item

            required property var model
            required isSelected
            required property string name
            required property string description
            required property bool isSoloist
            required property int index
            required property int sharedGroupRole
            required property string sharedGroupId
            required property bool isNew // added in this dialog, not yet in the score
            required property int tablatureOption // for new instruments with a tablature version (else -1)

            // Shared-staves groups, as in the layout panel: a header row and its instruments' rows, on one background
            readonly property bool isInGroup: sharedGroupRole !== LayoutPanelItemType.NOT_IN_GROUP
            readonly property bool isGroupHeader: sharedGroupRole === LayoutPanelItemType.GROUP_HEADER
            readonly property bool isGroupInstrument: isInGroup && !isGroupHeader
            readonly property bool isGroupBottom: sharedGroupRole === LayoutPanelItemType.GROUP_LAST_MEMBER

            // Groups are collapsed by default: then only their header is shown
            readonly property bool isGroupExpanded: isInGroup && Boolean(prv.expandedGroups[sharedGroupId])
            readonly property bool isHidden: isGroupInstrument && !isGroupExpanded
            readonly property bool closesGroup: isGroupBottom || (isGroupHeader && !isGroupExpanded)

            readonly property int contentTop: isGroupHeader ? prv.groupPadding : 0
            readonly property int contentBottom: !isInGroup ? 0 : isGroupHeader || isGroupBottom ? prv.groupPadding : prv.groupMemberSpacing
            readonly property int gap: isInGroup && !closesGroup ? 0 : prv.itemSpacing

            height: isHidden ? 0 : contentTop + prv.rowContentHeight + contentBottom + gap
            visible: !isHidden

            // The group's header shows the group's state on the whole group, instead of on its own row
            background.anchors.fill: contentArea
            background.visible: !isGroupHeader
            radius: 3
            mouseArea.anchors.fill: contentArea

            navigation.name: name
            navigation.enabled: !isHidden
            navigation.panel: instrumentsView.navigation
            navigation.row: 1 + index
            navigation.column: 0
            navigation.accessible.name: itemTitleLabel.text
            navigation.accessible.description: description
            navigation.accessible.row: index

            // Group hover: any of the group's rows being hovered
            readonly property bool isRowHovered: mouseArea.containsMouse
            property bool countsGroupHover: false

            function updateGroupHover() {
                const counts = isInGroup && isRowHovered
                if (counts !== countsGroupHover) {
                    prv.changeGroupHover(sharedGroupId, counts ? 1 : -1)
                    countsGroupHover = counts
                }
            }

            onIsRowHoveredChanged: updateGroupHover()
            onIsSelectedChanged: {
                if (isGroupHeader) {
                    prv.setGroupSelected(sharedGroupId, isSelected)
                }
            }

            Component.onCompleted: {
                if (isGroupHeader && isSelected) {
                    prv.setGroupSelected(sharedGroupId, true)
                }
            }

            Component.onDestruction: {
                if (countsGroupHover) {
                    prv.changeGroupHover(sharedGroupId, -1)
                }
                if (isGroupHeader) {
                    prv.setGroupSelected(sharedGroupId, false)
                }
            }

            // Group background: one segment per row, rounded at the group's ends
            Item {
                width: parent.width
                height: parent.height - item.gap
                z: -2

                visible: item.isInGroup
                clip: true

                Rectangle {
                    readonly property int overflow: 8
                    readonly property bool isSelected: Boolean(prv.selectedGroups[item.sharedGroupId])
                    readonly property bool isHovered: (prv.groupHoverCounts[item.sharedGroupId] || 0) > 0
                    readonly property bool isPressed: prv.pressedGroupId !== "" && prv.pressedGroupId === item.sharedGroupId

                    width: parent.width
                    y: item.isGroupHeader ? 0 : -overflow
                    height: parent.height + (item.isGroupHeader ? 0 : overflow) + (item.closesGroup ? 0 : overflow)

                    radius: 6
                    color: isSelected ? ui.theme.accentColor : ui.theme.buttonColor
                    opacity: isSelected ? (isPressed ? 0.7 : isHovered ? 0.4 : 0.5)
                                        : (isPressed ? 0.7 : isHovered ? 0.3 : 0.5)
                }

                // High contrast: group stroke (the theme's border width is only non-zero in high contrast themes)
                Rectangle {
                    readonly property int overflow: 8

                    visible: ui.theme.borderWidth > 0

                    width: parent.width
                    y: item.isGroupHeader ? 0 : -overflow
                    height: parent.height + (item.isGroupHeader ? 0 : overflow) + (item.closesGroup ? 0 : overflow)

                    radius: 6
                    color: "transparent"
                    border.width: ui.theme.borderWidth
                    border.color: ui.theme.strokeColor
                }
            }

            // Instruments in a group: their card, on the group's background
            Rectangle {
                anchors.fill: contentArea
                z: -1

                visible: item.isGroupInstrument
                radius: item.radius
                color: ui.theme.backgroundPrimaryColor
                border.width: ui.theme.borderWidth
                border.color: ui.theme.strokeColor
            }

            Item {
                id: contentArea

                x: item.isInGroup ? prv.groupPadding : 0
                y: item.contentTop
                width: item.width - x * 2
                height: prv.rowContentHeight

                FlatButton {
                    id: expandButton

                    width: prv.rowContentHeight
                    height: prv.rowContentHeight

                    visible: item.isGroupHeader

                    navigation.name: item.name + "Expand"
                    navigation.panel: instrumentsView.navigation
                    navigation.row: 1 + item.index
                    navigation.column: 2
                    navigation.accessible.name: item.isGroupExpanded
                                                //: Collapse a tree item
                                                ? qsTrc("global", "Collapse")
                                                //: Expand a tree item
                                                : qsTrc("global", "Expand")

                    transparent: true
                    icon: item.isGroupExpanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT

                    onClicked: {
                        prv.setGroupExpanded(item.sharedGroupId, !item.isGroupExpanded)
                    }
                }

                StyledTextLabel {
                    id: itemTitleLabel
                    anchors.left: parent.left
                    // A group's name follows its expand button; its instruments (which can't be expanded) line up with it
                    anchors.leftMargin: item.isInGroup ? prv.rowContentHeight + 4 : 12
                    anchors.right: tablatureDropdown.visible ? tablatureDropdown.left : parent.right
                    anchors.rightMargin: 4
                    anchors.verticalCenter: parent.verticalCenter

                    horizontalAlignment: Text.AlignLeft
                    text: item.isSoloist ? qsTrc("instruments", "Soloist:") + " " + item.name : item.name
                    // Instruments on combined shared staves are shown as regular text under their (bold) group;
                    // instruments that are only added once the dialog is accepted are shown in italics
                    font: {
                        const font = item.isGroupInstrument ? ui.theme.bodyFont : ui.theme.bodyBoldFont
                        return item.isNew && root.highlightsNewInstruments ? Qt.font(Object.assign({}, font, { italic: true })) : font
                    }
                }

                // Notation type, for new instruments that have a tablature version: which of their (linked) notation and
                // tablature staves are shown in the score
                StyledDropdown {
                    id: tablatureDropdown

                    anchors.right: parent.right
                    anchors.rightMargin: 4
                    anchors.verticalCenter: parent.verticalCenter

                    width: 112
                    height: 24

                    visible: item.tablatureOption >= 0

                    navigation.name: item.name + "NotationType"
                    navigation.panel: instrumentsView.navigation
                    navigation.row: 1 + item.index
                    navigation.column: 1
                    navigation.accessible.name: itemTitleLabel.text + " " + qsTrc("instruments", "notation type")

                    contentItem: Item {
                        property string text: ""
                        property alias labelItem: labelItem

                        StyledTextLabel {
                            id: labelItem

                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.right: dropIconItem.left
                            anchors.leftMargin: 8
                            anchors.rightMargin: 6

                            horizontalAlignment: Text.AlignLeft
                            text: parent.text
                        }

                        StyledIconLabel {
                            id: dropIconItem

                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 4

                            iconCode: IconCode.SMALL_ARROW_DOWN
                        }
                    }

                    model: [
                        qsTrc("instruments", "Notation"),
                        qsTrc("instruments", "Tablature"),
                        qsTrc("instruments", "Tab & notation")
                    ]
                    currentIndex: Math.max(item.tablatureOption, 0)

                    onActivated: function(index, value) {
                        item.model.tablatureOption = index

                        // Choosing an option selects the row
                        if (!item.isSelected) {
                            root.instrumentsOnScoreModel.selectRow(item.index)
                        }
                    }
                }

                // A group's header hides its own background (the group shows its state), and with it the usual
                // focus border, so it has one of its own
                NavigationFocusBorder {
                    navigationCtrl: item.navigation
                    drawOutsideParent: false
                    visible: item.isGroupHeader && item.navigation.highlight
                    radius: item.radius
                }
            }

            onPressed: function(isPressed) {
                if (isGroupHeader) {
                    prv.pressedGroupId = isPressed ? sharedGroupId : ""
                }
            }

            onClicked: {
                root.instrumentsOnScoreModel.selectRow(index)
            }

            onDoubleClicked: {
                root.instrumentsOnScoreModel.removeSelection()
            }

            onRemoveSelectionRequested: {
                root.instrumentsOnScoreModel.removeSelection()
            }
        }
    }

    StyledTextLabel {
        anchors.top: operationsRow.bottom
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        visible: instrumentsView.count === 0

        text: qsTrc("instruments", "Choose your instruments by adding them to this list")
        wrapMode: Text.WordWrap
        maximumLineCount: 2
    }
}
