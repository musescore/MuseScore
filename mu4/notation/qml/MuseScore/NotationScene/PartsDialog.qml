import QtQuick 2.9
import QtQuick.Controls 2.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

QmlDialog {
    width: 600
    height: 370

    modal: true

    Rectangle {
        anchors.fill: parent

        color: ui.theme.popupBackgroundColor

        PartListModel {
            id: partsModel
        }

        Component.onCompleted: {
            partsModel.load()
        }

        Column {
            anchors.fill: parent
            anchors.margins: 20

            spacing: 30

            Item {
                anchors.left: parent.left
                anchors.right: parent.right

                height: childrenRect.height

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
                        partsModel.createNewPart()
                    }
                }

                FlatButton {
                    id: deleteButton

                    anchors.right: parent.right

                    icon: IconCode.DELETE_TANK

                    onClicked: {

                    }
                }
            }

            ListView {
                id: view

                height: 200
                width: parent.width

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

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                    }

                    states: [
                        State {
                            name: "SELECTED"
                            when: mouseArea.pressed

                            PropertyChanges {
                                target: partRect
                                color: ui.theme.accentColor
                                opacity: ui.theme.buttonOpacityHit
                            }
                        }
                    ]
                }
            }

            FlatButton {
                anchors.right: parent.right

                text: qsTrc("notation", "Open all parts")

                onClicked: {
                    partsModel.openAllParts()
                }
            }
        }
    }
}
