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

    anchors.fill: parent

    //TODO: #22050 needed for this
    /*
    property Component toolbarComponent: PercussionPanelToolBar {
        navigation.section: root.navigationSection
        navigation.order: root.contentNavigationPanelOrderStart

        model: percussionPanelModel

        panelWidth: root.width
    }
    */

    Component.onCompleted: {
        padGrid.model.init()
    }

    PercussionPanelModel {
        id: percModel

        Component.onCompleted: {
            percModel.init()
        }
    }

    // TODO: Will live inside percussion panel until #22050 is implemented
    PercussionPanelToolBar {
        id: toolbar

        anchors.top: parent.top

        width: parent.width
        height: 36

        navigationSection: root.navigationSection
        navigationOrderStart: root.contentNavigationPanelOrderStart

        model: percModel

        panelWidth: root.width
    }

    StyledFlickable {
        id: flickable

        anchors.top: toolbar.bottom
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        width: Math.min(rowLayout.width, parent.width)

        contentWidth: rowLayout.width
        contentHeight: rowLayout.height

        function goToBottom() {
            var endY = flickable.contentHeight * (1.0 - flickable.visibleArea.heightRatio)
            flickable.contentY = endY
        }

        RowLayout {
            id: rowLayout

            // side columns being the "delete row" buttons on the left, and the "add row" button on the right
            readonly property int sideColumnsWidth: addRowButton.width

            height: padGrid.cellHeight * padGrid.numRows
            spacing: padGrid.spacing / 2

            NavigationPanel {
                id: deleteButtonsPanel

                name: "PercussionPanelDeleteRowButtons"
                section: root.navigationSection
                order: toolbar.navigationOrderEnd + 3

                enabled: deleteButtonsColumn.visible
            }

            Column {
                id: deleteButtonsColumn

                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true

                width: rowLayout.sideColumnsWidth

                visible: percModel.currentPanelMode === PanelMode.EDIT_LAYOUT

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

                            visible: padGrid.numRows > 1 && padGrid.model.rowIsEmpty(model.index)

                            icon: IconCode.DELETE_TANK
                            backgroundRadius: deleteButton.width / 2

                            navigation.panel: deleteButtonsPanel
                            navigation.row: model.index

                            onClicked: {
                                padGrid.model.deleteRow(model.index)
                            }

                            Connections {
                                target: padGrid.model

                                function onRowIsEmptyChanged(row, isEmpty) {
                                    if (row !== model.index) {
                                        return
                                    }
                                    deleteButton.visible = padGrid.numRows > 1 && isEmpty
                                }
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

                QtObject {
                    id: gridPrv

                    // This variable ensures we stay within a given pad when tabbing back-and-forth between
                    // "main" and "footer" controls
                    property var currentPadNavigationIndex: [0, 0]
                }

                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true

                width: cellWidth * numColumns

                interactive: false

                cellWidth: 100 + padGrid.spacing
                cellHeight: 100 + padGrid.spacing

                model: percModel.padListModel

                NavigationPanel {
                    id: padsNavPanel

                    name: "PercussionPanelPads"
                    section: root.navigationSection
                    order: toolbar.navigationOrderEnd + 1

                    onNavigationEvent: function(event) {
                        if (event.type === NavigationEvent.AboutActive) {
                            event.setData("controlIndex", gridPrv.currentPadNavigationIndex)
                        }
                    }
                }

                NavigationPanel {
                    id: padFootersNavPanel

                    name: "PercussionPanelFooters"
                    section: root.navigationSection
                    order: toolbar.navigationOrderEnd + 2

                    enabled: percModel.currentPanelMode !== PanelMode.EDIT_LAYOUT

                    onNavigationEvent: function(event) {
                        if (event.type === NavigationEvent.AboutActive) {
                            event.setData("controlIndex", gridPrv.currentPadNavigationIndex)
                        }
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

                        // When swapping, only show the outline for the swap origin  and the swap target...
                        showEditOutline: percModel.currentPanelMode === PanelMode.EDIT_LAYOUT
                                         && (!Boolean(padGrid.swapOriginPad) || padGrid.swapOriginPad === pad || pad.containsDrag)
                        showOriginBackground: pad.containsDrag || pad === padGrid.swapOriginPad

                        dragParent: root

                        navigationRow: index / padGrid.numColumns
                        navigationColumn: index % padGrid.numColumns
                        padNavigation.panel: padsNavPanel
                        footerNavigation.panel: padFootersNavPanel

                        onStartPadSwapRequested: {
                            padGrid.swapOriginPad = pad
                            padGrid.model.startPadSwap(index)
                        }

                        onDropped: function(dropEvent) {
                            padGrid.swapOriginPad = null
                            padGrid.model.endPadSwap(index)
                            dropEvent.accepted = true
                        }

                        onCancelPadSwapRequested: {
                            padGrid.swapOriginPad = null
                            padGrid.model.endPadSwap(-1)
                        }

                        onHasActiveControlChanged: {
                            if (!pad.hasActiveControl) {
                                return;
                            }
                            gridPrv.currentPadNavigationIndex = [pad.navigationRow, pad.navigationColumn]
                        }
                    }

                    states: [
                        // If this is the swap target - move the swappable area to the swap origin (preview the swap)
                        State {
                            name: "SWAP_TARGET"
                            when: Boolean(padGrid.swapOriginPad) && pad.containsDrag && padGrid.swapOriginPad !== pad
                            ParentChange {
                                target: pad.swappableArea
                                parent: padGrid.swapOriginPad
                            }
                            AnchorChanges {
                                target: pad.swappableArea
                                anchors.verticalCenter: padGrid.swapOriginPad.verticalCenter
                                anchors.horizontalCenter: padGrid.swapOriginPad.horizontalCenter
                            }
                            // Origin background not needed for the dragged pad when a preview is taking place...
                            PropertyChanges {
                                target: padGrid.swapOriginPad
                                showOriginBackground: false
                            }
                        }
                    ]
                }
            }

            NavigationPanel {
                id: addRowButtonPanel

                name: "PercussionPanelAddRowButton"
                section: root.navigationSection
                order: toolbar.navigationOrderEnd + 4

                enabled: addRowButton.visible
            }

            FlatButton {
                id: addRowButton

                // Display to the right of the last pad
                Layout.alignment: Qt.AlignBottom
                Layout.bottomMargin: (padGrid.cellHeight / 2) - (height / 2)

                visible: percModel.currentPanelMode === PanelMode.EDIT_LAYOUT

                icon: IconCode.PLUS
                text: qsTrc("notation", "Add row")
                orientation: Qt.Horizontal

                navigation.panel: addRowButtonPanel

                onClicked: {
                    padGrid.model.addEmptyRow()
                    flickable.goToBottom()
                }
            }
        }

        StyledTextLabel {
            id: panelDisabledLabel

            visible: !percModel.enabled

            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: (padGrid.cellHeight / 2) - (panelDisabledLabel.height / 2)

            font: ui.theme.bodyFont
            text: qsTrc("notation", "Select an unpitched percussion staff to use available sounds")
        }
    }
}
