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

    // italics until accepted (off when all instruments are new)
    property bool highlightsNewInstruments: false

    function addInstruments(instrumentIdList) {
        const contentY = instrumentsView.contentY
        const row = root.instrumentsOnScoreModel.addInstruments(instrumentIdList)

        // The model is reset, which scrolls the list back to the top
        instrumentsView.contentY = contentY
        instrumentsView.forceLayout()

        if (row >= 0) {
            instrumentsView.positionViewAtIndex(row, ListView.Contain)
        }
    }

    QtObject {
        id: prv

        // group id -> expanded; collapsed by default
        property var expandedGroups: ({})

        // becomes null by itself when the row is destroyed (e.g. scrolled out of view)
        property InstrumentsOnScoreDelegate hoveredGroupRow: null
        property string pressedGroupId: ""

        function setGroupRowHovered(row, hovered) {
            // cleared only by its own row: the next row can be entered before the previous one is left
            if (hovered) {
                prv.hoveredGroupRow = row
            } else if (prv.hoveredGroupRow === row) {
                prv.hoveredGroupRow = null
            }
        }

        function setGroupPressed(groupId, pressed) {
            if (pressed) {
                prv.pressedGroupId = groupId
            } else if (prv.pressedGroupId === groupId) {
                prv.pressedGroupId = ""
            }
        }

        function setGroupExpanded(groupId, expanded) {
            const groups = Object.assign({}, prv.expandedGroups)
            groups[groupId] = expanded
            prv.expandedGroups = groups
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

        // on while all selected are soloists
        FlatButton {
            id: makeSoloistButton

            Layout.preferredWidth: makeSoloistButton.width

            navigation.name: "MakeSoloist"
            navigation.panel: instrumentsView.navigation
            navigation.row: 0
            navigation.column: 1
            navigation.accessible.name: makeSoloistButton.toolTipTitle

            icon: IconCode.ACCOUNT
            accentButton: root.instrumentsOnScoreModel.isSelectionSoloist
            toolTipTitle: root.instrumentsOnScoreModel.isSelectionSoloist ? qsTrc("instruments", "Undo soloist")
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
                root.instrumentsOnScoreModel.removeSelectedBlocks()
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

        delegate: InstrumentsOnScoreDelegate {
            id: item

            navigationPanel: instrumentsView.navigation
            highlightsNewInstruments: root.highlightsNewInstruments

            isGroupExpanded: Boolean(prv.expandedGroups[item.sharedGroupId])

            // from the model: the header's row may not exist; hasSelection re-evaluates it on selection changes
            isGroupSelected: {
                root.instrumentsOnScoreModel.hasSelection
                return item.isInGroup && root.instrumentsOnScoreModel.isSharedGroupSelected(item.sharedGroupId)
            }

            isGroupHovered: Boolean(prv.hoveredGroupRow) && prv.hoveredGroupRow.sharedGroupId === item.sharedGroupId
            isGroupPressed: item.isInGroup && prv.pressedGroupId === item.sharedGroupId

            onClicked: {
                root.instrumentsOnScoreModel.selectRow(item.index)
            }

            onRemoveSelectionRequested: {
                root.instrumentsOnScoreModel.removeSelectedBlocks()
            }

            onGroupExpandToggled: {
                prv.setGroupExpanded(item.sharedGroupId, !item.isGroupExpanded)
            }

            onGroupHoverChanged: function(hovered) {
                prv.setGroupRowHovered(item, hovered)
            }

            onGroupPressChanged: function(pressed) {
                prv.setGroupPressed(item.sharedGroupId, pressed)
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
