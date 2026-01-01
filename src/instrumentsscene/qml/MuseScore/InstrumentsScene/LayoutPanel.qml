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

        LegacyTreeView {
            id: layoutPanelTreeView

            readonly property real delegateHeight: 38

            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: !treeModel.isEmpty

            model: LayoutPanelTreeModel {
                id: treeModel
            }

            selection: treeModel ? treeModel.selectionModel() : null

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

                rowDelegate: Item {
                    height: layoutPanelTreeView.delegateHeight
                    width: parent.width
                }
            }

            itemDelegate: DropArea {
                id: dropArea

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

                            sideMargin: contentColumn.sideMargin

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

                            onDoubleClicked: {
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
                            title: model?.item?.title || ""
                            isSelected: model?.item?.isSelected || false

                            navigation.panel: layoutPanelTreeView.navigationTreePanel
                            navigation.row: model?.index || 0

                            depth: styleData.depth
                            sideMargin: contentColumn.sideMargin

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
