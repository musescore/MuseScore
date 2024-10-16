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
        padGrid.model.load()
    }

    PercussionPanelModel {
        id: percModel
    }

    // TODO: Will live inside percussion panel until #22050 is implemented
    PercussionPanelToolBar {
        id: toolbar

        anchors.top: parent.top

        width: parent.width
        height: 36

        navigation.section: root.navigationSection
        navigation.order: root.contentNavigationPanelOrderStart

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

        RowLayout {
            id: rowLayout

            // side columns being the "delete row" buttons on the left, and the "add row" button on the right
            readonly property int sideColumnsWidth: addRowButton.width

            height: padGrid.cellHeight * padGrid.numRows
            spacing: padGrid.spacing / 2

            Column {
                id: deleteButtonsColumn

                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true

                width: rowLayout.sideColumnsWidth

                visible: percModel.currentPanelMode === PanelMode.EDIT_LAYOUT

                Repeater {
                    model: padGrid.numRows

                    delegate: Item {
                        id: deleteButtonArea

                        width: parent.width
                        height: padGrid.cellHeight

                        FlatButton {
                            id: deleteButton

                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right

                            visible: model.index > 0

                            icon: IconCode.DELETE_TANK
                            backgroundRadius: deleteButton.width / 2

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
                readonly property int spacing: 20

                property Item draggedPad: null

                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true

                width: cellWidth * numColumns

                interactive: false

                cellWidth: 100 + padGrid.spacing
                cellHeight: 100 + padGrid.spacing

                model: percModel.padListModel

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
                        panelMode: percModel.currentPanelMode
                        useNotationPreview: percModel.useNotationPreview

                        showOriginBackground: pad.containsDrag || pad === padGrid.draggedPad

                        dragParent: root

                        onDragStarted: {
                            padGrid.draggedPad = pad
                            padGrid.model.startDrag(index)
                        }

                        onDropped: function(dropEvent) {
                            padGrid.draggedPad = null
                            padGrid.model.endDrag(index)
                            dropEvent.accepted = true
                        }

                        onDragCancelled: {
                            padGrid.draggedPad = null
                            padGrid.model.endDrag(-1)
                        }
                    }

                    states: [
                        // If this is the drop target - move the draggable area to the origin of the dragged pad (preview the drop)
                        State {
                            name: "DROP_TARGET"
                            when: Boolean(padGrid.draggedPad) && pad.containsDrag && padGrid.draggedPad !== pad
                            ParentChange {
                                target: pad.draggableArea
                                parent: padGrid.draggedPad
                            }
                            AnchorChanges {
                                target: pad.draggableArea
                                anchors.verticalCenter: padGrid.draggedPad.verticalCenter
                                anchors.horizontalCenter: padGrid.draggedPad.horizontalCenter
                            }
                            // Origin background not needed for the dragged pad when a preview is taking place...
                            PropertyChanges {
                                target: padGrid.draggedPad
                                showOriginBackground: false
                            }
                        }
                    ]
                }
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
                onClicked: {
                    padGrid.model.addRow()
                }
            }
        }
    }
}

