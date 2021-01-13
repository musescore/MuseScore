import QtQuick 2.9
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

import "internal"

QmlDialog {
    id: root

    width: 280
    height: 600

    modal: true

    Rectangle {
        id: contentRect

        anchors.fill: parent

        color: ui.theme.popupBackgroundColor

        NoteInputBarCustomiseModel {
            id: customiseModel
        }

        Component.onCompleted: {
            customiseModel.load()
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12

            spacing: 0

            StyledTextLabel {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                Layout.topMargin: 8

                text: qsTrc("notation", "Customise toolbar")
                horizontalAlignment: Text.AlignLeft
                font: ui.theme.largeBodyBoldFont
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 20

                height: childrenRect.height

                FlatButton {
                    Layout.fillWidth: true

                    text: qsTrc("notation", "Add separator line")

                    enabled: customiseModel.isAddSeparatorAvailable

                    onClicked: {
                        customiseModel.addSeparatorLine()
                    }
                }

                FlatButton {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 30

                    icon: IconCode.DELETE_TANK
                    enabled: customiseModel.isRemovingAvailable

                    onClicked: {
                        customiseModel.removeSelectedRows()
                    }
                }

                FlatButton {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 30

                    icon: IconCode.ARROW_UP
                    enabled: customiseModel.isMovingUpAvailable

                    onClicked: {
                        customiseModel.moveSelectedRowsUp()
                        Qt.callLater(view.positionViewAtSelectedItems)
                    }
                }

                FlatButton {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 30

                    icon: IconCode.ARROW_DOWN
                    enabled: customiseModel.isMovingDownAvailable

                    onClicked: {
                        customiseModel.moveSelectedRowsDown()
                        Qt.callLater(view.positionViewAtSelectedItems)
                    }
                }
            }

            ListView {
                id: view

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: 12

                spacing: 0

                model: customiseModel

                boundsBehavior: Flickable.StopAtBounds
                clip: true

                function positionViewAtSelectedItems() {
                    var selectedIndexes = customiseModel.selectionModel.selectedIndexes
                    for (var _index in selectedIndexes) {
                        positionViewAtIndex(selectedIndexes[_index].row, ListView.Contain)
                    }
                }

                ScrollBar.vertical: StyledScrollBar {

                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right

                    visible: view.contentHeight > view.height
                    z: 1
                }

                delegate: ListItemBlank {
                    height: 38

                    isSelected: selectedRole

                    onClicked: {
                        customiseModel.selectRow(index)
                    }

                    Loader {
                        property var delegateType: Boolean(itemRole) ? itemRole.type : NoteInputBarItem.UNDEFINED

                        height: parent.height
                        width: parent.width

                        sourceComponent: delegateType === NoteInputBarItem.ACTION ? actionComponent : separatorLineComponent

                        Component {
                            id: actionComponent

                            NoteInputBarActionDelegate {}
                        }

                        Component {
                            id: separatorLineComponent

                            StyledTextLabel {
                                anchors.centerIn: parent
                                text: Boolean(itemRole) ? itemRole.title : ""
                            }
                        }
                    }
                }
            }
        }
    }
}
