import QtQuick 2.9
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQml.Models 2.11

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

QmlDialog {
    id: root

    width: 600
    height: 370

    modal: true

    Rectangle {
        anchors.fill: parent

        color: ui.theme.popupBackgroundColor

        PartListModel {
            id: partsModel
        }

        ItemSelectionModel {
            id: selectionModel
            model: partsModel
        }

        Component.onCompleted: {
            partsModel.load()
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20

            spacing: 30

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: childrenRect.height

                StyledTextLabel {
                    anchors.left: parent.left

                    text: qsTrc("notation", "Parts")

                    font.pixelSize: 18
                }

                FlatButton {
                    text: qsTrc("notation", "Create new part")

                    anchors.right: deleteButton.left
                    anchors.rightMargin: 8

                    onClicked: {
                        partsModel.apply()
                        partsModel.createNewPart()
                        root.hide()
                    }
                }

                FlatButton {
                    id: deleteButton

                    anchors.right: parent.right

                    icon: IconCode.DELETE_TANK

                    onClicked: {
                        partsModel.removeParts(selectionModel.selectedIndexes)
                        selectionModel.clear()
                    }
                }
            }

            ListView {
                id: view

                Layout.preferredHeight: 200
                Layout.fillWidth: true

                spacing: 0

                model: partsModel

                boundsBehavior: Flickable.StopAtBounds
                clip: true

                readonly property int margin: 26

                header: Item {
                    width: parent.width
                    height: childrenRect.height

                    Column {
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

                delegate: Rectangle {
                    id: partRect

                    width: parent.width
                    height: 40

                    color: "transparent"
                    opacity: 1

                    StyledTextLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: view.margin

                        text: model.title

                        font.bold: true
                    }

                    SeparatorLine { anchors.bottom: parent.bottom }

                    property bool selected: false

                    Connections {
                        target: selectionModel

                        function onHasSelectionChanged() {
                            if (!selectionModel.hasSelection) {
                                selected = false
                            }
                        }
                    }

                    MouseArea {
                        id: mouseArea

                        anchors.fill: parent

                        onClicked: {
                            var index = partsModel.index(model.row, 0)
                            selectionModel.select(index, ItemSelectionModel.Select)
                            selected = !selected
                        }
                    }

                    states: [
                        State {
                            name: "SELECTED"
                            when: selected

                            PropertyChanges {
                                target: partRect
                                color: ui.theme.accentColor
                                opacity: ui.theme.buttonOpacityHit
                            }
                        }
                    ]
                }
            }

            Row {
                Layout.preferredHeight: childrenRect.height
                Layout.alignment: Qt.AlignRight

                spacing: 16

                FlatButton {
                    text: qsTrc("notation", "Cancel")

                    onClicked: {
                        root.reject()
                    }
                }

                FlatButton {
                    text: qsTrc("notation", "Open")

                    onClicked: {
                        partsModel.apply()
                        partsModel.openParts(selectionModel.selectedIndexes)
                        root.hide()
                    }
                }
            }
        }
    }
}
