import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

QmlDialog {
    id: root

    height: 300
    width: 700

    Rectangle {
        id: rect

        anchors.fill: parent
        color: ui.theme.backgroundColor

        property int currentGroupIndex: -1
        property int currentInstrumentIndex: -1

        InstrumentListModel {
            id: instrumentsModel
        }

        Component.onCompleted: {
            instrumentsModel.load()
        }

        StyledComboBox {
            id: ganres

            textRoleName: "text"
            valueRoleName: "value"

            model: {
                var resultList = []

                var families = instrumentsModel.families

                for (var i = 0; i < families.length; ++i) {
                    resultList.push({"text" : families[i].name, "value" : families[i].id})
                }

                return resultList
            }

            onValueChanged: {
                instrumentsModel.selectFamily(value)
            }
        }

        ListView {
            id: groupsView

            anchors.top: ganres.bottom
            anchors.bottom: parent.bottom
            anchors.topMargin: 10
            width: 100

            model: instrumentsModel.groups

            delegate: Rectangle {
                width: parent.width
                height: 30

                border.color: "black"
                border.width: 1
                color: rect.currentGroupIndex == index ? "green" : ui.theme.backgroundColor

                StyledTextLabel {
                    anchors.fill: parent

                    text: modelData.name
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        rect.currentGroupIndex = index
                        instrumentsModel.selectGroup(modelData.id)
                    }
                }
            }
        }

        ListView {
            id: instrumentsView

            anchors.top: groupsView.top
            anchors.bottom: parent.bottom
            anchors.left: groupsView.right
            anchors.rightMargin: 10
            width: 100

            model: instrumentsModel.instruments

            delegate: Rectangle {
                width: parent.width
                height: 30

                border.color: "black"
                border.width: 1
                color: rect.currentInstrumentIndex == index ? "green" : ui.theme.backgroundColor

                StyledTextLabel {
                    anchors.fill: parent

                    text: modelData.name
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        rect.currentInstrumentIndex = index
                    }
                }
            }
        }
    }
}
