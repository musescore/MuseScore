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
    height: 825

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
                font.pixelSize: 15
                font.bold: true
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
                    }
                }

                FlatButton {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 30

                    icon: IconCode.ARROW_DOWN
                    enabled: customiseModel.isMovingDownAvailable

                    onClicked: {
                        customiseModel.moveSelectedRowsDown()
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

                ScrollBar.vertical: StyledScrollBar {

                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right

                    visible: view.contentHeight > view.height
                    z: 1
                }

                delegate: Item {
                    width: parent ? parent.width : 0
                    height: 48

                    Loader {
                        property var delegateType: Boolean(itemRole) ? itemRole.type : NoteInputBarItem.UNDEFINED

                        height: parent.height
                        width: parent.width

                        sourceComponent: delegateType === NoteInputBarItem.ACTION ? actionComponent : separatorLineComponent

                        Component {
                            id: actionComponent

                            NoteInputBarActionDelegate {

                                onClicked: {
                                    customiseModel.selectRow(index)
                                }
                            }
                        }

                        Component {
                            id: separatorLineComponent

                            NoteInputBarSeparatorDelegate {

                                onClicked: {
                                    customiseModel.selectRow(index)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
