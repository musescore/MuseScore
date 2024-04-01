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
            instrumentsTreeView.expandCollapseAll(expand)
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
            navigation.order: root.navigationOrderStart

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

        StyledFlickable {
            id: flickable

            Layout.fillWidth: true
            Layout.fillHeight: true

            contentWidth: width
            contentHeight: instrumentsTreeView.implicitHeight

            TreeView {
                id: instrumentsTreeView

                readonly property real delegateHeight: 38

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                implicitHeight: flickableItem.contentHeight
                flickableItem.interactive: false

                visible: !instrumentsTreeModel.isEmpty

                model: InstrumentsPanelTreeModel {
                    id: instrumentsTreeModel
                }

                selection: instrumentsTreeModel ? instrumentsTreeModel.selectionModel() : null

                alternatingRowColors: false
                backgroundVisible: false
                headerVisible: false
                frameVisible: false

                function expandCollapseAll(expand) {
                    for (let row = 0; row < instrumentsTreeView.model.rowCount(); row++) {
                        const instrumentIndex = instrumentsTreeView.model.index(row, 0);
                        const instrumentsTreeItemDelegate = instrumentsTreeView.model.data(instrumentIndex);
                        if (instrumentsTreeItemDelegate.isExpandable){
                            if (expand) {
                                instrumentsTreeView.expand(instrumentIndex)
                            } else {
                                instrumentsTreeView.collapse(instrumentIndex)
                            }
                        }
                    }
                    flickable.returnToBounds();
                }

                function scrollToFocusedItem(focusedIndex) {
                    let targetScrollPosition = focusedIndex * instrumentsTreeView.delegateHeight
                    let visibleAreaEnd = flickable.contentY + flickable.height

                    if (targetScrollPosition + instrumentsTreeView.delegateHeight > visibleAreaEnd) {
                        flickable.contentY = Math.min(targetScrollPosition + instrumentsTreeView.delegateHeight - flickable.height, flickable.contentHeight - flickable.height)
                    } else if (targetScrollPosition < flickable.contentY) {
                        flickable.contentY = Math.max(targetScrollPosition, 0)
                    }
                }

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

                TableViewColumn {
                    role: "itemRole"
                }

                function isControl(itemType) {
                    return itemType === InstrumentsTreeItemType.CONTROL_ADD_STAFF ||
                            itemType === InstrumentsTreeItemType.CONTROL_ADD_DOUBLE_INSTRUMENT
                }

                style: TreeViewStyle {
                    indentation: 0

                    frame: Item {}
                    incrementControl: Item {}
                    decrementControl: Item {}
                    handle: Item {}
                    scrollBarBackground: Item {}
                    branchDelegate: Item {}

                    backgroundColor: "transparent"

                    rowDelegate: Item {
                        height: instrumentsTreeView.delegateHeight
                        width: parent.width
                    }
                }

                itemDelegate: DropArea {
                    id: dropArea

                    Loader {
                        id: treeItemDelegateLoader

                        property int delegateType: model ? model.itemRole.type : InstrumentsTreeItemType.UNDEFINED

                        height: parent.height
                        width: parent.width

                        sourceComponent: instrumentsTreeView.isControl(delegateType) ?
                                             controlItemDelegateComponent : treeItemDelegateComponent

                        Component {
                            id: treeItemDelegateComponent

                            InstrumentsTreeItemDelegate {
                                treeView: instrumentsTreeView
                                item: model ? model.itemRole : null
                                originalParent: treeItemDelegateLoader

                                sideMargin: contentColumn.sideMargin
                                popupAnchorItem: root

                                navigation.name: model ? model.itemRole.title : "ItemInstrumentsTree"
                                navigation.panel: instrumentsTreeView.navigationTreePanel
                                navigation.row: model ? model.index : 0
                                navigation.onActiveChanged: {
                                    if (navigation.active) {
                                        prv.currentItemNavigationName = navigation.name
                                        instrumentsTreeView.scrollToFocusedItem(model.index)
                                    }
                                }

                                onClicked: {
                                    instrumentsTreeModel.selectRow(styleData.index)
                                }

                                onDoubleClicked: {
                                    if (!isExpandable) {
                                        return
                                    }

                                    if (!styleData.isExpanded) {
                                        instrumentsTreeView.expand(styleData.index)
                                    } else {
                                        instrumentsTreeView.collapse(styleData.index)
                                    }
                                }

                                onRemoveSelectionRequested: {
                                    instrumentsTreeModel.removeSelectedRows()
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

                                onVisibilityChanged: function(visible) {
                                    instrumentsTreeModel.toggleVisibilityOfSelectedRows(visible);
                                }

                                onDragStarted: {
                                    instrumentsTreeModel.startActiveDrag()
                                }

                                onDropped: {
                                    instrumentsTreeModel.endActiveDrag()
                                }
                            }
                        }

                        Component {
                            id: controlItemDelegateComponent

                            InstrumentsTreeItemControl {
                                isSelected: model ? model.itemRole.isSelected : false

                                navigation.panel: instrumentsTreeView.navigationTreePanel
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

                        instrumentsTreeModel.moveRows(drag.source.index.parent,
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
