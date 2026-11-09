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

        ToggleButton {
            Layout.leftMargin: contentColumn.sideMargin

            text: qsTrc("layoutpanel", "Enable stave sharing")
            checked: treeModel.isStaveSharingEnabled
            onToggled: treeModel.toggleStaveSharing(!checked)
        }

        SeparatorLine {}

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

            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: !treeModel.isEmpty

            model: LayoutPanelTreeModel {
                id: treeModel
            }

            columnWidthProvider: function(column) {
                return layoutPanelTreeView.width
            }

            function expandCollapseAll(expand) {
                for (let i = 0; i < treeModel.rowCount(); ++i) {
                    const modelIndex = treeModel.index(i, 0)
                    if (!treeModel.modelIndexToItem(modelIndex).isExpandable) {
                        continue
                    }

                    const row = layoutPanelTreeView.rowAtIndex(modelIndex)
                    if (expand) {
                        layoutPanelTreeView.expand(row)
                    } else {
                        layoutPanelTreeView.collapse(row)
                    }
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

            function isControl(itemType) {
                return itemType === LayoutPanelItemType.CONTROL_ADD_STAFF
            }

            delegate: DropArea {
                id: dropArea

                required property TreeView treeView
                required property int row
                required property int column
                required property int depth
                required property bool expanded
                required property var model

                readonly property var modelIndex: dropArea.treeView.index(dropArea.row, dropArea.column)
                readonly property AbstractLayoutPanelTreeItem item: dropArea.model.item ?? null

                implicitHeight: 38

                Loader {
                    id: treeItemDelegateLoader

                    property int delegateType: dropArea.item ? dropArea.item.type : LayoutPanelItemType.UNDEFINED

                    anchors.fill: parent

                    sourceComponent: layoutPanelTreeView.isControl(delegateType) ?
                                         controlItemDelegateComponent : treeItemDelegateComponent

                    Component {
                        id: treeItemDelegateComponent

                        LayoutPanelItemDelegate {
                            id: itemDelegate

                            treeView: layoutPanelTreeView
                            item: dropArea.item
                            modelIndex: dropArea.modelIndex
                            row: dropArea.row
                            depth: dropArea.depth
                            isExpanded: dropArea.expanded

                            sideMargin: contentColumn.sideMargin

                            navigation.name: item?.title || "LayoutPanelItemDelegate"
                            navigation.panel: layoutPanelTreeView.navigationTreePanel
                            navigation.row: dropArea.row
                            navigation.onActiveChanged: {
                                if (navigation.active) {
                                    prv.currentItemNavigationName = navigation.name
                                    layoutPanelTreeView.positionViewAtRow(dropArea.row, TableView.Contain)
                                }
                            }

                            onClicked: {
                                if (itemDelegate.isSelectable) {
                                    treeModel.selectRow(dropArea.modelIndex)
                                }
                            }

                            onDoubleClicked: {
                                if (itemDelegate.isExpandable) {
                                    layoutPanelTreeView.toggleExpanded(dropArea.row)
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
                                layoutPanelTreeView.interactive = !itemDelegate.isPopupOpened
                            }
                        }
                    }

                    Component {
                        id: controlItemDelegateComponent

                        LayoutPanelItemControl {
                            title: dropArea.item?.title || ""
                            isSelected: dropArea.item?.isSelected || false

                            navigation.panel: layoutPanelTreeView.navigationTreePanel
                            navigation.row: dropArea.row

                            depth: dropArea.depth
                            sideMargin: contentColumn.sideMargin

                            onClicked: {
                                dropArea.item.appendNewItem()
                            }
                        }
                    }
                }

                onEntered: function(drag) {
                    const draggedItem = drag.source as LayoutPanelItemDelegate
                    if (!draggedItem) {
                        return
                    }

                    if (dropArea.modelIndex === draggedItem.modelIndex || !dropArea.item.canAcceptDrop(draggedItem.item)) {
                        return
                    }

                    if (draggedItem.modelIndex.row < 0 || dropArea.modelIndex.row < 0) {
                        return;
                    }

                    Qt.callLater(treeModel.moveRows,
                                 draggedItem.modelIndex.parent,
                                 draggedItem.modelIndex.row,
                                 1,
                                 dropArea.modelIndex.parent,
                                 dropArea.modelIndex.row)
                }
            }
        }
    }
}
