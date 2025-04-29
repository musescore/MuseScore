/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
import QtQuick 2.15
import QtQuick.Layouts

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

import "internal"

Item {
    id: root

    property NavigationSection navigationSection: null
    property int contentNavigationPanelOrderStart: 1

    signal resizeRequested(var newWidth, var newHeight)

    anchors.fill: parent

    property Component toolbarComponent: PercussionPanelToolBar {
        navigationSection: root.navigationSection
        navigationOrderStart: root.contentNavigationPanelOrderStart

        model: percModel

        panelWidth: root.width
    }

    function resizePanelToContentHeight() {
        var newHeight = (Math.min(padGrid.numRows, 2) * padGrid.cellHeight) + (soundTitleLabel.height * 2)
        root.resizeRequested(root.width, newHeight)
    }

    Component.onCompleted: {
        padGrid.model.init()
        root.resizePanelToContentHeight()
    }

    PercussionPanelModel {
        id: percModel

        Component.onCompleted: {
            percModel.init()
        }

        onCurrentPanelModeChanged: {
            // Cancel any active keyboard swaps when the panel mode changes
            if (padGrid.isKeyboardSwapActive) {
                padGrid.swapOriginPad = null
                padGrid.isKeyboardSwapActive = false
                padGrid.model.endPadSwap(-1)
            }
        }
    }

    StyledIconLabel {
        id: soundTitleIcon

        anchors.verticalCenter: soundTitleLabel.verticalCenter
        anchors.right: soundTitleLabel.left

        anchors.rightMargin: 6

        visible: percModel.enabled && !percModel.soundTitle.isEmpty

        color: ui.theme.fontPrimaryColor

        iconCode: IconCode.AUDIO
    }

    StyledTextLabel {
        id: soundTitleLabel

        anchors {
            top: root.top
            right: root.right

            bottomMargin: 8
            rightMargin: 16
        }

        visible: percModel.enabled && !percModel.soundTitle.isEmpty

        text: percModel.soundTitle
    }

    StyledFlickable {
        id: flickable

        anchors.top: soundTitleLabel.bottom
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        width: Math.min(rowLayout.width, parent.width)

        contentWidth: rowLayout.width
        contentHeight: rowLayout.height + rowLayout.anchors.topMargin

        StyledScrollBar.vertical: verticalScrollBar

        function goToBottom() {
            var endY = flickable.contentHeight * (1.0 - flickable.visibleArea.heightRatio)
            flickable.contentY = endY
        }

        RowLayout {
            id: rowLayout

            // side columns being the "delete row" buttons on the left, and the "add row" button on the right
            readonly property int sideColumnsWidth: addRowButton.width

            QtObject {
                id: navigationPrv

                // This variable ensures we stay within a given pad when tabbing back-and-forth between "main" and
                // "footer" controls. It's also used to tab to the associated delete button for a given empty row
                property var currentPadNavigationIndex: [0, 0]

                function onPadNavigationEvent(event) {
                    var navigationRow = navigationPrv.currentPadNavigationIndex[0]
                    var navigationColumn = navigationPrv.currentPadNavigationIndex[1]

                    if (navigationRow >= padGrid.numRows || navigationColumn >= padGrid.numColumns) {
                        navigationPrv.currentPadNavigationIndex = [0, 0]
                    }

                    if (event.type === NavigationEvent.AboutActive) {
                        event.setData("controlIndex", navigationPrv.currentPadNavigationIndex)
                    }
                }
            }

            height: padGrid.cellHeight * padGrid.numRows
            anchors.top: parent.top
            anchors.topMargin: Math.max((flickable.height - height) / 2, 0)
            spacing: padGrid.spacing / 2

            NavigationPanel {
                id: deleteButtonsPanel

                name: "PercussionPanelDeleteRowButtons"
                section: root.navigationSection
                order: padFootersNavPanel.order + 1

                enabled: deleteButtonsColumn.visible

                onNavigationEvent: {
                    // Use the last known "pad navigation row" and tab to the associated delete button if it exists
                    var padNavigationRow = navigationPrv.currentPadNavigationIndex[0]
                    if (padGrid.numRows > 1) {
                        event.setData("controlIndex", [padNavigationRow, 0])
                    }
                }
            }

            Column {
                id: deleteButtonsColumn

                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true

                width: rowLayout.sideColumnsWidth

                visible: percModel.currentPanelMode === PanelMode.EDIT_LAYOUT
                enabled: !padGrid.isKeyboardSwapActive

                Repeater {
                    id: deleteRepeater

                    model: padGrid.numRows

                    delegate: Item {
                        id: deleteButtonArea

                        width: parent.width
                        height: padGrid.cellHeight

                        FlatButton {
                            id: deleteButton

                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right

                            visible: padGrid.numRows > 1

                            icon: IconCode.DELETE_TANK
                            backgroundRadius: deleteButton.width / 2

                            navigation {
                                panel: deleteButtonsPanel
                                row: model.index
                                column: 0
                            }

                            onClicked: {
                                padGrid.model.deleteRow(model.index)
                            }
                        }
                    }
                }
            }

            GridView {
                id: padGrid

                readonly property int numRows: Math.floor(model.numPads / numColumns)
                readonly property int numColumns: model.numColumns
                readonly property int spacing: 12

                property Item swapOriginPad: null
                property bool isKeyboardSwapActive: false

                width: cellWidth * numColumns
                Layout.fillHeight: true

                interactive: false

                cellWidth: 100 + padGrid.spacing
                cellHeight: 100 + padGrid.spacing

                model: percModel.padListModel

                NavigationPanel {
                    id: padsNavPanel

                    name: "PercussionPanelPads"
                    section: root.navigationSection
                    order: root.contentNavigationPanelOrderStart + 2 // +2 for toolbar

                    onNavigationEvent: function(event) {
                        navigationPrv.onPadNavigationEvent(event)
                    }
                }

                NavigationPanel {
                    id: padFootersNavPanel

                    name: "PercussionPanelFooters"
                    section: root.navigationSection
                    order: padsNavPanel.order + 1

                    enabled: percModel.currentPanelMode !== PanelMode.EDIT_LAYOUT

                    onNavigationEvent: function(event) {
                        navigationPrv.onPadNavigationEvent(event)
                    }
                }

                delegate: Item {
                    id: padArea

                    width: padGrid.cellWidth
                    height: padGrid.cellHeight

                    PercussionPanelPad {
                        id: pad

                        anchors.centerIn: parent

                        width: parent.width + pad.totalBorderWidth - padGrid.spacing
                        height: parent.height + pad.totalBorderWidth - padGrid.spacing

                        padModel: model.padModelRole
                        panelEnabled: percModel.enabled
                        panelMode: percModel.currentPanelMode
                        useNotationPreview: percModel.useNotationPreview
                        notationPreviewNumStaffLines: percModel.notationPreviewNumStaffLines

                        // When swapping, only show the outline for the swap origin  and the swap target...
                        showEditOutline: percModel.currentPanelMode === PanelMode.EDIT_LAYOUT
                                         && (!Boolean(padGrid.swapOriginPad) || padGrid.swapOriginPad === pad)
                        showOriginBackground: pad.containsDrag || (pad === padGrid.swapOriginPad && !padGrid.isKeyboardSwapActive)

                        panelHasActiveKeyboardSwap: padGrid.isKeyboardSwapActive
                        dragParent: root

                        navigationRow: index / padGrid.numColumns
                        navigationColumn: index % padGrid.numColumns
                        padNavigation.panel: padsNavPanel
                        footerNavigation.panel: padFootersNavPanel

                        onStartPadSwapRequested: function(isKeyboardSwap) {
                            padGrid.swapOriginPad = pad
                            padGrid.isKeyboardSwapActive = isKeyboardSwap
                            padGrid.model.startPadSwap(index)
                            if (isKeyboardSwap) {
                                pad.padNavigation.requestActive()
                            }
                        }

                        onEndPadSwapRequested: {
                            padGrid.swapOriginPad = null
                            padGrid.isKeyboardSwapActive = false
                            padGrid.model.endPadSwap(index)
                        }

                        onCancelPadSwapRequested: {
                            padGrid.swapOriginPad = null
                            padGrid.isKeyboardSwapActive = false
                            padGrid.model.endPadSwap(-1)
                        }

                        onHasActiveControlChanged: {
                            if (!pad.hasActiveControl) {
                                return;
                            }
                            navigationPrv.currentPadNavigationIndex = [pad.navigationRow, pad.navigationColumn]
                        }

                        Connections {
                            target: padGrid.model

                            function onPadFocusRequested(padIndex) {
                                if (index !== padIndex) {
                                    return
                                }

                                // Focus pad only if keyboard navigation has started
                                if (root.navigationSection.active) {
                                    pad.padNavigation.requestActive()
                                }
                            }

                            function onNumPadsChanged() {
                                root.resizePanelToContentHeight()
                            }
                        }
                    }

                    states: [
                        // If this is the swap target - move the swappable area to the swap origin (preview the swap)
                        State {
                            name: "SWAP_TARGET"
                            when: Boolean(padGrid.swapOriginPad) && (pad.containsDrag || pad.padNavigation.active) && padGrid.swapOriginPad !== pad

                            ParentChange {
                                target: pad.swappableArea
                                parent: padGrid.swapOriginPad
                            }
                            AnchorChanges {
                                target: pad.swappableArea
                                anchors.verticalCenter: padGrid.swapOriginPad.verticalCenter
                                anchors.horizontalCenter: padGrid.swapOriginPad.horizontalCenter
                            }
                            PropertyChanges {
                                target: pad
                                showEditOutline: true
                            }

                            // Origin background not needed for the dragged pad when a preview is taking place...
                            PropertyChanges {
                                target: padGrid.swapOriginPad
                                showOriginBackground: false
                            }

                            // In the case of a keyboard swap, we also need to move the origin pad
                            ParentChange {
                                target: padGrid.isKeyboardSwapActive && Boolean(padGrid.swapOriginPad) ? padGrid.swapOriginPad.swappableArea : null
                                parent: pad
                            }
                            AnchorChanges {
                                target: padGrid.isKeyboardSwapActive && Boolean(padGrid.swapOriginPad) ? padGrid.swapOriginPad.swappableArea : null
                                anchors.verticalCenter: pad.verticalCenter
                                anchors.horizontalCenter: pad.horizontalCenter
                            }
                        }
                    ]
                }
            }

            NavigationPanel {
                id: addRowButtonPanel

                name: "PercussionPanelAddRowButton"
                section: root.navigationSection
                order: deleteButtonsPanel.order + 1

                enabled: addRowButton.visible
            }

            FlatButton {
                id: addRowButton

                // Display to the right of the last pad
                Layout.alignment: Qt.AlignBottom
                Layout.bottomMargin: (padGrid.cellHeight / 2) - (height / 2)

                visible: percModel.currentPanelMode === PanelMode.EDIT_LAYOUT
                enabled: !padGrid.isKeyboardSwapActive

                icon: IconCode.PLUS
                text: qsTrc("notation/percussion", "Add row")
                orientation: Qt.Horizontal

                navigation.panel: addRowButtonPanel
                drawFocusBorderInsideRect: true

                onClicked: {
                    padGrid.model.addEmptyRow(/*focusFirstInNewRow*/ true)
                    flickable.goToBottom()
                }
            }
        }

        StyledTextLabel {
            id: panelDisabledLabel

            visible: !percModel.enabled

            anchors.centerIn: parent
            anchors.verticalCenterOffset: rowLayout.anchors.topMargin / 2

            font: ui.theme.bodyFont
            text: qsTrc("notation/percussion", "Select an unpitched percussion staff to see available sounds")
        }
    }

    StyledScrollBar {
        id: verticalScrollBar
        height: root.height
        anchors.right: root.right
    }
}
