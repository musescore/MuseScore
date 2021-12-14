/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
import QtQuick.Controls
import QtQuick.Layouts

import MuseScore.Ui
import MuseScore.UiComponents
import MuseScore.InstrumentsScene

import "internal"

Item {
    id: root

    property NavigationSection navigationSection: null
    property NavigationPanel navigationPanel: controlPanel.navigation // first panel
    property alias contextMenuModel: contextMenuModel

    onVisibleChanged: {
        instrumentsTreeModel.setInstrumentsPanelVisible(root.visible)
    }

    Rectangle {
        id: background

        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        MouseArea {
            anchors.fill: parent

            onClicked: {
                instrumentsTreeModel.clearSelection()
            }
        }
    }

    InstrumentsPanelContextMenuModel {
        id: contextMenuModel

        onExpandCollapseAllRequested: function(expand) {
            expand ? instrumentsTreeView.expandRecursively() : instrumentsTreeView.collapseRecursively()
        }
    }

    Component.onCompleted: {
        contextMenuModel.load()
    }

    QtObject {
        id: prv

        property string currentItemNavigationName: ""
    }

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent

        readonly property int sideMargin: 12
        spacing: sideMargin

        InstrumentsControlPanel {
            id: controlPanel
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            Layout.leftMargin: contentColumn.sideMargin
            Layout.rightMargin: contentColumn.sideMargin

            navigation.section: root.navigationSection
            navigation.order: 2

            isMovingUpAvailable: instrumentsTreeModel.isMovingUpAvailable
            isMovingDownAvailable: instrumentsTreeModel.isMovingDownAvailable
            isAddingAvailable: instrumentsTreeModel.isAddingAvailable
            isRemovingAvailable: instrumentsTreeModel.isRemovingAvailable
            isInstrumentSelected: instrumentsTreeModel.isInstrumentSelected

            onAddRequested: {
                instrumentsTreeModel.addInstruments()
            }

            onMoveUpRequested: {
                instrumentsTreeModel.moveSelectedRowsUp()
            }

            onMoveDownRequested: {
                instrumentsTreeModel.moveSelectedRowsDown()
            }

            onRemovingRequested: {
                instrumentsTreeModel.removeSelectedRows()
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 12
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            text: Boolean(instrumentsTreeModel.addInstrumentsKeyboardShortcut)
                  //: Keep in sync with the text of the "Add" button at the top of the Instruments panel (InstrumentsControlPanel.qml)
                  ? qsTrc("instruments", "There are no instruments in your score. To choose some, press <b>Add</b>, or use the keyboard shortcut %1.")
                    .arg("<b>" + instrumentsTreeModel.addInstrumentsKeyboardShortcut + "</b>")
                  //: Keep in sync with the text of the "Add" button at the top of the Instruments panel (InstrumentsControlPanel.qml)
                  : qsTrc("instruments", "There are no instruments in your score. To choose some, press <b>Add</b>.")
            visible: instrumentsTreeModel.isEmpty && instrumentsTreeModel.isAddingAvailable

            verticalAlignment: Qt.AlignTop
            wrapMode: Text.WordWrap
        }

        TreeView {
            id: instrumentsTreeView

            Layout.fillWidth: true
            Layout.fillHeight: true

            contentWidth: width

            visible: !instrumentsTreeModel.isEmpty

            clip: true
            boundsBehavior: Flickable.StopAtBounds
            maximumFlickVelocity: ui.theme.flickableMaxVelocity

            ScrollBar.vertical: StyledScrollBar {}
            ScrollBar.horizontal: StyledScrollBar {}

            model: InstrumentsPanelTreeModel {
                id: instrumentsTreeModel
            }

            selectionModel: instrumentsTreeModel ? instrumentsTreeModel.selectionModel() : null

            property NavigationPanel navigationTreePanel : NavigationPanel {
                name: "InstrumentsTree"
                section: root.navigationSection
                direction: NavigationPanel.Both
                enabled: instrumentsTreeView.enabled && instrumentsTreeView.visible
                order: controlPanel.navigation.order + 1

                onNavigationEvent: function(event) {
                    if (event.type === NavigationEvent.AboutActive) {
                        event.setData("controlName", prv.currentItemNavigationName)
                    }
                }
            }

            function isControl(itemType) {
                return itemType === InstrumentsTreeItemType.CONTROL_ADD_STAFF
                        || itemType === InstrumentsTreeItemType.CONTROL_ADD_DOUBLE_INSTRUMENT
            }

            delegate: DropArea {
                id: dropArea

                required property bool expanded
                required property int depth
                required property int row
                required property int column

                readonly property var item: model ? model.itemRole : null
                readonly property var modelIndex: instrumentsTreeView.modelIndex(row, column)

                width: parent.width
                implicitHeight: 38

                Loader {
                    id: treeItemDelegateLoader

                    readonly property alias item: dropArea.item
                    readonly property var delegateType: item ? item.type : InstrumentsTreeItemType.UNDEFINED

                    anchors.fill: parent

                    sourceComponent: instrumentsTreeView.isControl(delegateType)
                                     ? controlItemDelegateComponent : treeItemDelegateComponent

                    Component {
                        id: treeItemDelegateComponent

                        InstrumentsTreeItemDelegate {
                            treeView: instrumentsTreeView

                            item: dropArea.item
                            row: dropArea.row
                            modelIndex: dropArea.modelIndex
                            isExpanded: dropArea.expanded
                            depth: dropArea.depth

                            sideMargin: contentColumn.sideMargin
                            popupAnchorItem: root

                            navigation.panel: instrumentsTreeView.navigationTreePanel
                            navigation.row: dropArea.row
                            navigation.onActiveChanged: {
                                if (navigation.active) {
                                    prv.currentItemNavigationName = navigation.name
                                }

                                onDragStarted: {
                                    instrumentsTreeModel.startActiveDrag()
                                }

                                onDropped: {
                                    instrumentsTreeModel.endActiveDrag()
                                }
                            }

                            onClicked: {
                                instrumentsTreeModel.selectRow(dropArea.modelIndex)
                            }

                            onToggleExpandedRequested: {
                                if (!isExpandable) {
                                    return
                                }

                                instrumentsTreeView.toggleExpanded(dropArea.row)
                            }

                            onVisibilityChanged: function(visible) {
                                instrumentsTreeModel.toggleVisibilityOfSelectedRows(visible);
                            }

                            onRemoveSelectionRequested: {
                                instrumentsTreeModel.removeSelectedRows()
                            }

                            property real contentYBackup: 0

                            onPopupOpened: function(popupX, popupY, popupHeight) {
                                contentYBackup = instrumentsTreeView.contentY
                                var mappedPopupY = mapToItem(instrumentsTreeView, popupX, popupY).y

                                if (mappedPopupY + popupHeight < instrumentsTreeView.height - contentColumn.sideMargin) {
                                    return
                                }

                                var hiddenPopupPartHeight = Math.abs(instrumentsTreeView.height - (mappedPopupY + popupHeight))
                                instrumentsTreeView.contentY += hiddenPopupPartHeight + contentColumn.sideMargin
                            }

                            onPopupClosed: {
                                instrumentsTreeView.contentY = contentYBackup
                            }
                        }
                    }

                    Component {
                        id: controlItemDelegateComponent

                        InstrumentsTreeItemControl {
                            item: treeItemDelegateLoader.item
                            depth: dropArea.depth

                            navigation.panel: instrumentsTreeView.navigationTreePanel
                            navigation.row: dropArea.row

                            sideMargin: contentColumn.sideMargin
                        }
                    }
                }

                onEntered: function(drag) {
                    if (modelIndex === drag.source.modelIndex || !model.itemRole.canAcceptDrop(drag.source.item)) {
                        return
                    }

                    instrumentsTreeModel.moveRows(drag.source.modelIndex.parent,
                                                  drag.source.modelIndex.row,
                                                  1,
                                                  modelIndex.parent,
                                                  modelIndex.row)
                }
            }
        }
    }
}
