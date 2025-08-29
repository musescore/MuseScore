/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui
import Muse.UiComponents
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
    }

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent

        readonly property int sideMargin: 12
        spacing: sideMargin

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

        StyledTreeView {
            id: layoutPanelTreeView

            readonly property real delegateHeight: 38

            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: !treeModel.isEmpty

            model: LayoutPanelTreeModel {
                id: treeModel
            }

            columnWidthProvider: function (column) {
                return width
            }

            // Sad for performance, but doesn't work well with drag & drop
            reuseItems: false

            function expandCollapseAll(expand) {
                for (let modelRow = 0; modelRow < layoutPanelTreeView.model.rowCount(); ++modelRow) {
                    const modelIndex = layoutPanelTreeView.model.index(modelRow, 0);
                    const isExpandable = layoutPanelTreeView.model.data(modelIndex).isExpandable;
                    if (!isExpandable) {
                        continue;
                    }
                    const viewRow = layoutPanelTreeView.rowAtIndex(modelRow);
                    if (expand) {
                        layoutPanelTreeView.expand(viewRow)
                    } else {
                        layoutPanelTreeView.collapse(viewRow)
                    }
                }
                returnToBounds();
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

            delegate: DropArea {
                id: dropArea

                // Assigned to by TreeView:
                required property TreeView treeView
                required property bool isTreeNode
                required property bool expanded
                required property bool hasChildren
                required property int depth
                required property int row
                required property int column
                required property bool current

                readonly property var modelIndex: treeView.index(row, column)

                required property QtObject item

                implicitWidth: parent.width
                implicitHeight: layoutPanelTreeView.delegateHeight

                Loader {
                    id: treeItemDelegateLoader

                    property int delegateType: dropArea.item.type

                    anchors.fill: parent

                    sourceComponent: {
                        switch (dropArea.item.type) {
                        case LayoutPanelItemType.CONTROL_ADD_STAFF:
                        case LayoutPanelItemType.CONTROL_ADD_DOUBLE_INSTRUMENT:
                            return controlItemDelegateComponent;
                        default:
                            return treeItemDelegateComponent;
                        }
                    }

                    Component {
                        id: treeItemDelegateComponent

                        LayoutPanelItemDelegate {
                            id: itemDelegate

                            treeView: dropArea.treeView
                            isTreeNode: dropArea.isTreeNode
                            expanded: dropArea.expanded
                            hasChildren: dropArea.hasChildren
                            depth: dropArea.depth
                            row: dropArea.row
                            column: dropArea.column
                            current: dropArea.current
                            modelIndex: dropArea.modelIndex
                            item: dropArea.item

                            sideMargin: contentColumn.sideMargin
                            popupAnchorItem: root

                            navigation.name: item.title
                            navigation.panel: layoutPanelTreeView.navigationTreePanel
                            navigation.row: row
                            navigation.onActiveChanged: {
                                if (navigation.active) {
                                    prv.currentItemNavigationName = navigation.name
                                    treeView.positionViewAtRow(row)
                                }
                            }

                            onClicked: {
                                if (isSelectable) {
                                    treeModel.selectRow(modelIndex)
                                }
                            }

                            onDoubleClicked: {
                                if (!isExpandable) {
                                    return
                                }

                                treeView.toggleExpanded(row)
                            }

                            onRemoveSelectionRequested: {
                                treeModel.removeSelectedRows()
                            }

                            property real contentYBackup: 0

                            onPopupOpened: function(popupX, popupY, popupHeight) {
                                contentYBackup = layoutPanelTreeView.contentY
                                var mappedPopupY = mapToItem(layoutPanelTreeView, popupX, popupY).y

                                if (mappedPopupY + popupHeight < layoutPanelTreeView.height - contentColumn.sideMargin) {
                                    return
                                }

                                var hiddenPopupPartHeight = Math.abs(layoutPanelTreeView.height - (mappedPopupY + popupHeight))
                                layoutPanelTreeView.contentY += hiddenPopupPartHeight + contentColumn.sideMargin
                            }

                            onPopupClosed: {
                                layoutPanelTreeView.contentY = contentYBackup
                            }

                            onChangeVisibilityOfSelectedRowsRequested: function(visible) {
                                treeModel.changeVisibilityOfSelectedRows(visible);
                            }

                            onChangeVisibilityRequested: function(visible) {
                                treeModel.changeVisibility(modelIndex, visible)
                            }

                            onDragStarted: {
                                treeModel.startActiveDrag()
                            }

                            onDropped: {
                                treeModel.endActiveDrag()
                            }
                        }
                    }

                    Component {
                        id: controlItemDelegateComponent

                        LayoutPanelItemControl {
                            treeView: dropArea.treeView
                            isTreeNode: dropArea.isTreeNode
                            expanded: dropArea.expanded
                            hasChildren: dropArea.hasChildren
                            depth: dropArea.depth
                            row: dropArea.row
                            column: dropArea.column
                            current: dropArea.current
                            modelIndex: dropArea.modelIndex
                            item: dropArea.item

                            isSelected: item.isSelected

                            navigation.panel: layoutPanelTreeView.navigationTreePanel
                            navigation.row: row

                            sideMargin: contentColumn.sideMargin

                            onClicked: {
                                item.appendNewItem()
                            }
                        }
                    }
                }

                onEntered: function(drag) {
                    // drag.source is the `LayoutPanelItemDelegate`
                    if (row === drag.source.row || !item.canAcceptDrop(drag.source.item)) {
                        return
                    }

                    if (drag.source.modelIndex.row < 0 || modelIndex.row < 0) {
                        return;
                    }

                    treeModel.moveRows(drag.source.modelIndex.parent,
                                       drag.source.modelIndex.row,
                                       1,
                                       modelIndex.parent,
                                       modelIndex.row)
                }
            }
        }
    }
}
