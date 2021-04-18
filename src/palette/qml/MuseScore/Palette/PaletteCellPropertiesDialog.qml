import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Palette 1.0

QmlDialog {
    id: root

    width: 280
    height: 370

    title: qsTrc("palette", "Palette Cell Properties")

    property var properties

    Rectangle {
        color: ui.theme.backgroundPrimaryColor

        PaletteCellPropertiesModel {
            id: propertiesModel
        }

        Component.onCompleted: {
            propertiesModel.load(root.properties)
        }

        Column {
            anchors.fill: parent

            readonly property int margins: 12
            anchors.margins: margins

            spacing: 12

            StyledTextLabel {
                text: qsTrc("palette", "Name")
                font: ui.theme.bodyBoldFont
            }

            TextInputField {
                currentText: propertiesModel.name

                onCurrentTextEdited: {
                    propertiesModel.name = newTextValue
                }
            }

            SeparatorLine { anchors.margins: -parent.margins }

            StyledTextLabel {
                text: qsTrc("palette", "Content offset")
                font: ui.theme.bodyBoldFont
            }

            Grid {
                width: parent.width

                columns: 2
                spacing: 16

                Repeater {
                    id: repeater

                    model: [
                        { title: qsTrc("palette", "X"), value: propertiesModel.xOffset, incrementStep: 1, measureUnit: qsTrc("palette", "sp") },
                        { title: qsTrc("palette", "Y"), value: propertiesModel.yOffset, incrementStep: 1, measureUnit: qsTrc("palette", "sp") },
                        { title: qsTrc("palette", "Content scale"), value: propertiesModel.scaleFactor, incrementStep: 0.1 }
                    ]

                    function setValue(index, value) {
                        if (index === 0) {
                            propertiesModel.xOffset = value
                        } else if (index === 1) {
                            propertiesModel.yOffset = value
                        } else if (index === 2) {
                            propertiesModel.scaleFactor = value
                        }
                    }

                    Column {
                        width: parent.width / 2 - 8

                        spacing: 8

                        StyledTextLabel {
                            text: modelData["title"]
                        }

                        IncrementalPropertyControl {
                            currentValue: modelData["value"]
                            measureUnitsSymbol: Boolean(modelData["measureUnit"]) ? modelData["measureUnit"] : ""
                            step: modelData["incrementStep"]

                            onValueEdited: {
                                repeater.setValue(model.index, newValue)
                            }
                        }
                    }
                }
            }

            CheckBox {
                text: qsTrc("palette", "Draw staff")

                checked: propertiesModel.drawStaff

                onClicked: {
                    propertiesModel.drawStaff = !checked
                }
            }

            Item { height: 1; width: parent.width }

            Row {
                width: parent.width
                height: childrenRect.height + 20

                spacing: 4

                FlatButton {
                    text: qsTrc("global", "Cancel")

                    width: parent.width / 2

                    onClicked: {
                        propertiesModel.reject()
                        root.hide()
                    }
                }

                FlatButton {
                    text: qsTrc("global", "OK")

                    width: parent.width / 2

                    onClicked: {
                        root.hide()
                    }
                }
            }
        }
    }
}
