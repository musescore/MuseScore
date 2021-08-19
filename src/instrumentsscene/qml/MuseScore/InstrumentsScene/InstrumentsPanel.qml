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
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

import "internal"

Item {
    id: root

    property NavigationSection navigationSection: null
    property alias contextMenuModel: contextMenuModel.items

    Rectangle {
        id: background

        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        MouseArea {
            anchors.fill: parent

            onClicked: {
                instrumentsTreeModel.selectionModel.clear()
            }
        }
    }

    NavigationPanel {
        id: navigationTreePanel
        name: "InstrumentsTree"
        section: root.navigationSection
        direction: NavigationPanel.Both
        enabled: root.visible
        order: 3
    }

    InstrumentsPanelContextMenuModel {
        id: contextMenuModel
    }

    Component.onCompleted: {
        contextMenuModel.load()
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
            navigation.enabled: root.visible
            navigation.order: 2

            isMovingUpAvailable: instrumentsTreeModel.isMovingUpAvailable
            isMovingDownAvailable: instrumentsTreeModel.isMovingDownAvailable
            isAddingAvailable: instrumentsTreeModel.isAddingAvailable
            isRemovingAvailable: instrumentsTreeModel.isRemovingAvailable

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

            text: qsTrc("instruments", "There are no instruments in your score. To choose some, press <b>Add</b>, or use the shortcut <b>‘i’</b>")
            visible: instrumentsTreeModel.isEmpty && instrumentsTreeModel.isAddingAvailable

            verticalAlignment: Qt.AlignTop
            wrapMode: Text.WordWrap
        }

        Flickable {
            id: flickable

            Layout.fillWidth: true
            Layout.fillHeight: true

            contentWidth: width
            contentHeight: instrumentsTreeView.height

            clip: true
            boundsBehavior: Flickable.StopAtBounds
            interactive: height < contentHeight

            ScrollBar.vertical: StyledScrollBar {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
            }

            TreeView {
                id: instrumentsTreeView

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: flickableItem.contentHeight + 2 //! HACK +2 needs for correct show focus border

                visible: !instrumentsTreeModel.isEmpty

                model: InstrumentsPanelTreeModel {
                    id: instrumentsTreeModel
                }

                selection: instrumentsTreeModel ? instrumentsTreeModel.selectionModel : null

                alternatingRowColors: false
                headerVisible: false
                frameVisible: false

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

                    backgroundColor: background.color

                    rowDelegate: Rectangle {
                        id: rowTreeDelegate

                        height: Boolean(model) && instrumentsTreeView.isControl(model.itemRole.type) ? 64 : 38
                        width: parent.width
                        color: ui.theme.strokeColor
                    }
                }

                itemDelegate: DropArea {
                    id: dropArea

                    property bool isSelectable: model ? model.itemRole.isSelectable : false

                    Loader {
                        id: treeItemDelegateLoader

                        property var delegateType: model ? model.itemRole.type : InstrumentsTreeItemType.UNDEFINED

                        height: parent.height
                        width: parent.width

                        sourceComponent: instrumentsTreeView.isControl(delegateType) ?
                                             controlItemDelegateComponent : treeItemDelegateComponent

                        property bool isSelected: false

                        function updateIsSelected() {
                            treeItemDelegateLoader.isSelected = instrumentsTreeModel.isSelected(styleData.index)
                        }

                        Connections {
                            target: instrumentsTreeModel

                            function onSelectionChanged() {
                                treeItemDelegateLoader.updateIsSelected()
                            }
                        }

                        Component {
                            id: treeItemDelegateComponent

                            InstrumentsTreeItemDelegate {
                                attachedControl: instrumentsTreeView
                                isSelected: treeItemDelegateLoader.isSelected

                                keynavRow: model ? model.index : 0
                                navigationPanel: navigationTreePanel

                                isDragAvailable: dropArea.isSelectable
                                type: treeItemDelegateLoader.delegateType
                                sideMargin: contentColumn.sideMargin

                                popupAnchorItem: root

                                onClicked: {
                                    instrumentsTreeModel.selectRow(styleData.index)
                                }

                                onDoubleClicked: {
                                    if (styleData.hasChildren
                                            && (root.type === InstrumentsTreeItemType.INSTRUMENT
                                                    ? styleData.index.row === 0 : true)) {
                                        if (!styleData.isExpanded) {
                                            instrumentsTreeView.expand(styleData.index)
                                        } else {
                                            instrumentsTreeView.collapse(styleData.index)
                                        }
                                    }
                                }

                                onFocusActived: {
                                    instrumentsTreeModel.selectRow(styleData.index)
                                }

                                onVisibleChanged: {
                                    treeItemDelegateLoader.updateIsSelected()
                                }

                                property real contentYBackup: 0

                                onPopupOpened: {
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
                            }
                        }

                        Component {
                            id: controlItemDelegateComponent

                            InstrumentsTreeItemControl {
                                isHighlighted: treeItemDelegateLoader.isSelected

                                keynavRow: model ? model.index : 0
                                navigationPanel: navigationTreePanel

                                onClicked: {
                                    styleData.value.appendNewItem()
                                }

                                onVisibleChanged: {
                                    treeItemDelegateLoader.updateIsSelected()
                                }
                            }
                        }
                    }

                    onEntered: {
                        if (styleData.index === drag.source.index || !styleData.value.canAcceptDrop(drag.source.type)) {
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
