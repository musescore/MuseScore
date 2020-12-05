import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

import "internal"

Item {
    id: root

    Rectangle {
        id: background

        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor
    }

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent

        readonly property int sideMargin: 12
        spacing: sideMargin

        InstrumentsControlPanel {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            Layout.leftMargin: contentColumn.sideMargin
            Layout.rightMargin: contentColumn.sideMargin

            isMovingUpAvailable: instrumentTreeModel.isMovingUpAvailable
            isMovingDownAvailable: instrumentTreeModel.isMovingDownAvailable
            isAddingAvailable: instrumentTreeModel.isAddingAvailable
            isRemovingAvailable: instrumentTreeModel.isRemovingAvailable

            onAddRequested: {
                instrumentTreeModel.addInstruments()
            }

            onMoveUpRequested: {
                instrumentTreeModel.moveSelectedRowsUp()
            }

            onMoveDownRequested: {
                instrumentTreeModel.moveSelectedRowsDown()
            }

            onRemovingRequested: {
                instrumentTreeModel.removeSelectedRows()
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 12
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            text: qsTrc("instruments", "There are no instruments in your score. To choose some, press <b>Add</b>, or use the shortcut <b>‘i’</b>")
            visible: instrumentTreeModel.isEmpty && instrumentTreeModel.isAddingAvailable

            verticalAlignment: Qt.AlignTop
            wrapMode: Text.WordWrap
        }

        TreeView {
            id: instrumentsTreeView

            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: !instrumentTreeModel.isEmpty

            model: InstrumentPanelTreeModel {
                id: instrumentTreeModel
            }

            selection: instrumentTreeModel ? instrumentTreeModel.selectionModel : null

            TableViewColumn {
                role: "itemRole"
            }

            function isControl(itemType) {
                return itemType === InstrumentTreeItemType.CONTROL_ADD_STAFF ||
                       itemType === InstrumentTreeItemType.CONTROL_ADD_DOUBLE_INSTRUMENT
            }

            style: TreeViewStyle {
                indentation: 0

                frame: Item {}
                incrementControl: Item {}
                decrementControl: Item {}

                backgroundColor: background.color

                readonly property int scrollBarWidth: 6

                scrollBarBackground: Item {
                    width: scrollBarWidth
                }

                handle: Rectangle {
                    width: scrollBarWidth
                    implicitWidth: scrollBarWidth

                    radius: 8

                    color: ui.theme.fontPrimaryColor
                    opacity: styleData.pressed ? 0.7 : 0.3
                }

                rowDelegate: Rectangle {
                    id: rowTreeDelegate

                    height: Boolean(model) && instrumentsTreeView.isControl(model.itemRole.type) ? 64 : 38
                    width: parent.width
                    color: ui.theme.strokeColor
                }

                branchDelegate: Item {}
            }

            itemDelegate: DropArea {
                id: dropArea

                property bool isSelectable: model ? model.itemRole.isSelectable : false

                Loader {
                    id: treeItemDelegateLoader

                    property var delegateType: model ? model.itemRole.type : InstrumentTreeItemType.UNDEFINED

                    height: parent.height
                    width: parent.width

                    sourceComponent: instrumentsTreeView.isControl(delegateType) ?
                                     controlItemDelegateComponent : treeItemDelegateComponent

                    property bool isSelected: false

                    function updateIsSelected() {
                        treeItemDelegateLoader.isSelected = instrumentTreeModel.isSelected(styleData.index)
                    }

                    Connections {
                        target: instrumentTreeModel

                        function onSelectionChanged() {
                            treeItemDelegateLoader.updateIsSelected()
                        }
                    }

                    Component {
                        id: treeItemDelegateComponent

                        InstrumentsTreeItemDelegate {
                            attachedControl: instrumentsTreeView
                            isSelected: treeItemDelegateLoader.isSelected

                            isDragAvailable: dropArea.isSelectable
                            type: treeItemDelegateLoader.delegateType
                            sideMargin: contentColumn.sideMargin

                            onClicked: {
                                var isMultipleSelectionModeOn = mouse.modifiers & Qt.ShiftModifier || mouse.modifiers & Qt.ControlModifier

                                instrumentTreeModel.selectRow(styleData.index, isMultipleSelectionModeOn)
                            }

                            onVisibleChanged: {
                                treeItemDelegateLoader.updateIsSelected()
                            }
                        }
                    }

                    Component {
                        id: controlItemDelegateComponent

                        InstrumentsTreeItemControl {
                            isHighlighted: treeItemDelegateLoader.isSelected

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

                    instrumentTreeModel.moveRows(drag.source.index.parent,
                                                 drag.source.index.row,
                                                 1,
                                                 styleData.index.parent,
                                                 styleData.index.row)
                }
            }

            alternatingRowColors: false
            headerVisible: false
        }
    }
}
