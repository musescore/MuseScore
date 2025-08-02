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
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

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

            text: Boolean(treeModel.addInstrumentsKeyboardShortcut)
                  //: Keep in sync with the text of the "Add" button at the top of the Layout panel (LayoutControlPanel.qml)
                  ? qsTrc("layoutpanel", "There are no instruments in your score. To choose some, press <b>Add</b>, or use the keyboard shortcut %1.")
                    .arg("<b>" + treeModel.addInstrumentsKeyboardShortcut + "</b>")
                  //: Keep in sync with the text of the "Add" button at the top of the Layout panel (LayoutControlPanel.qml)
                  : qsTrc("layoutpanel", "There are no instruments in your score. To choose some, press <b>Add</b>.")
            visible: treeModel.isEmpty && treeModel.isAddingAvailable

            verticalAlignment: Qt.AlignTop
            wrapMode: Text.WordWrap
        }

        StyledFlickable {
            id: flickable

            Layout.fillWidth: true
            Layout.fillHeight: true

            contentWidth: width
            contentHeight: layoutPanelTreeView.implicitHeight

            LegacyTreeView {
                id: layoutPanelTreeView

                readonly property real delegateHeight: 38

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                implicitHeight: flickableItem.contentHeight
                flickableItem.interactive: false

                visible: !treeModel.isEmpty

                model: LayoutPanelTreeModel {
                    id: treeModel
                }

                selection: treeModel ? treeModel.selectionModel() : null

                alternatingRowColors: false
                backgroundVisible: false
                headerVisible: false
                frameVisible: false

                function expandCollapseAll(expand) {
                    for (let row = 0; row < layoutPanelTreeView.model.rowCount(); row++) {
                        const instrumentIndex = layoutPanelTreeView.model.index(row, 0);
                        const itemDelegate = layoutPanelTreeView.model.data(instrumentIndex);
                        if (itemDelegate.isExpandable){
                            if (expand) {
                                layoutPanelTreeView.expand(instrumentIndex)
                            } else {
                                layoutPanelTreeView.collapse(instrumentIndex)
                            }
                        }
                    }
                    flickable.returnToBounds();
                }

                function scrollToFocusedItem(focusedIndex) {
                    let targetScrollPosition = focusedIndex * layoutPanelTreeView.delegateHeight
                    let visibleAreaEnd = flickable.contentY + flickable.height

                    if (targetScrollPosition + layoutPanelTreeView.delegateHeight > visibleAreaEnd) {
                        flickable.contentY = Math.min(targetScrollPosition + layoutPanelTreeView.delegateHeight - flickable.height, flickable.contentHeight - flickable.height)
                    } else if (targetScrollPosition < flickable.contentY) {
                        flickable.contentY = Math.max(targetScrollPosition, 0)
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
                    role: "itemRole"
                }

                function isControl(itemType) {
                    return itemType === LayoutPanelItemType.CONTROL_ADD_STAFF ||
                           itemType === LayoutPanelItemType.CONTROL_ADD_DOUBLE_INSTRUMENT
                }

                style: LegacyTreeViewStyle {
                    indentation: 0

                    frame: Item {}
                    incrementControl: Item {}
                    decrementControl: Item {}
                    handle: Item {}
                    scrollBarBackground: Item {}
                    branchDelegate: Item {}

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

                        property int delegateType: model ? model.itemRole.type : LayoutPanelItemType.UNDEFINED

                        height: parent.height
                        width: parent.width

                        sourceComponent: layoutPanelTreeView.isControl(delegateType) ?
                                             controlItemDelegateComponent : treeItemDelegateComponent

                        Component {
                            id: treeItemDelegateComponent

                            LayoutPanelItemDelegate {
                                id: itemDelegate

                                treeView: layoutPanelTreeView
                                item: model ? model.itemRole : null
                                originalParent: treeItemDelegateLoader

                                sideMargin: contentColumn.sideMargin
                                popupAnchorItem: root

                                navigation.name: model ? model.itemRole.title : "LayoutPanelItemDelegate"
                                navigation.panel: layoutPanelTreeView.navigationTreePanel
                                navigation.row: model ? model.index : 0
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

                                property real contentYBackup: 0

                                onPopupOpened: function(popupX, popupY, popupHeight) {
                                    contentYBackup = flickable.contentY
                                    var mappedPopupY = mapToItem(flickable, popupX, popupY).y

                                    if (mappedPopupY + popupHeight < flickable.height - contentColumn.sideMargin) {
                                        return
                                    }

                                    var hiddenPopupPartHeight = Math.abs(flickable.height - (mappedPopupY + popupHeight))
                                    flickable.contentY += hiddenPopupPartHeight + contentColumn.sideMargin
                                }

                                onPopupClosed: {
                                    flickable.contentY = contentYBackup
                                }

                                onChangeVisibilityOfSelectedRowsRequested: function(visible) {
                                    treeModel.changeVisibilityOfSelectedRows(visible);
                                }

                                onChangeVisibilityRequested: function(index, visible) {
                                    treeModel.changeVisibility(index, visible)
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
                                isSelected: model ? model.itemRole.isSelected : false

                                navigation.panel: layoutPanelTreeView.navigationTreePanel
                                navigation.row: model ? model.index : 0

                                sideMargin: contentColumn.sideMargin

                                onClicked: {
                                    styleData.value.appendNewItem()
                                }
                            }
                        }
                    }

                    onEntered: function(drag) {
                        if (styleData.index === drag.source.index || !styleData.value.canAcceptDrop(drag.source.item)) {
                            return
                        }

                        if (drag.source.index.row < 0 || styleData.index.row < 0) {
                            return;
                        }

                        Qt.callLater(treeModel.moveRows,
                                     drag.source.index.parent,
                                     drag.source.index.row,
                                     1,
                                     styleData.index.parent,
                                     styleData.index.row)
                    }
                }
            }
        }
    }
}
