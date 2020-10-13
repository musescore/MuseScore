import QtQuick 2.9
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

QmlDialog {
    id: root

    width: 600
    height: 370

    modal: true

    Rectangle {
        id: contentRect

        anchors.fill: parent

        color: ui.theme.popupBackgroundColor

        MovementListModel {
            id: movementsModel
        }

        Component.onCompleted: {
            movementsModel.load()
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20

            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop

                height: childrenRect.height

                StyledTextLabel {
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true

                    text: qsTrc("notation", "Movements")
                    horizontalAlignment: Text.AlignLeft
                    font.pixelSize: 18
                }

                FlatButton {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 30

                    icon: IconCode.PLUS

                    onClicked: {
                        movementsModel.createNewMovement()
                    }
                }

                FlatButton {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 30

                    icon: IconCode.DELETE_TANK
                    enabled: movementsModel.isRemovingAvailable

                    onClicked: {
                        movementsModel.removeSelectedRows()
                    }
                }

                FlatButton {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 30

                    icon: IconCode.ARROW_DOWN
                    enabled: movementsModel.isMovingDownAvailable

                    onClicked: {
                        movementsModel.moveSelectedRowsDown()
                    }
                }

                FlatButton {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 30

                    icon: IconCode.ARROW_UP
                    enabled: movementsModel.isMovingUpAvailable

                    onClicked: {
                        movementsModel.moveSelectedRowsUp()
                    }
                }
            }

            Item {
                Layout.topMargin: 30
                Layout.fillWidth: true
                Layout.preferredHeight: nameColumn.height

                Column {
                    id: nameColumn

                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: view.spacing

                    width: parent.width

                    spacing: 16

                    StyledTextLabel {
                        anchors.left: parent.left
                        anchors.leftMargin: view.margin

                        text: qsTrc("notation", "Name")
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: 10
                    }

                    SeparatorLine {}
                }
            }

            ListView {
                id: view

                Layout.fillWidth: true
                Layout.fillHeight: true

                spacing: 0

                model: movementsModel

                boundsBehavior: Flickable.StopAtBounds
                clip: true

                readonly property int margin: 26

                delegate: Item {
                    width: parent ? parent.width : 0
                    height: 40

                    Rectangle {
                        id: background
                        anchors.fill: parent
                        color: "transparent"
                    }

                    StyledTextLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: view.margin

                        text: model.title

                        font.bold: true
                    }

                    SeparatorLine { anchors.bottom: parent.bottom }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent

                        onClicked: {
                            movementsModel.selectRow(index)
                        }
                    }

                    states: [
                        State {
                            name: "SELECTED"
                            when: model.selected

                            PropertyChanges {
                                target: background
                                color: ui.theme.accentColor
                                opacity: ui.theme.accentOpacityHit
                            }
                        }
                    ]
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBottom | Qt.AlignRight

                FlatButton {
                    Layout.alignment: Qt.AlignRight

                    text: qsTrc("notation", "Cancel")

                    onClicked: {
                        root.reject()
                    }
                }

                FlatButton {
                    Layout.alignment: Qt.AlignRight

                    text: qsTrc("notation", "Ok")

                    onClicked: {
                        root.hide()
                    }
                }
            }
        }
    }
}
