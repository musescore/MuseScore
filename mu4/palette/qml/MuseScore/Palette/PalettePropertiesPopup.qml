import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

StyledPopup {
    id: root

    property string paletteName: ""
    property bool showGrid: false
    property int cellWidth: 0
    property int cellHeight: 0
    property real elementOffset: 0
    property real scaleFactor: 0

    height: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        anchors.fill: parent
        anchors.margins: 12

        spacing: 12

        StyledTextLabel {
            text: qsTrc("palette", "Name")
            font.bold: true
        }

        TextInputField {
            id: paletteNameField

            currentText: root.paletteName

            onCurrentTextEdited: {
                root.paletteName = newTextValue
            }
        }

        SeparatorLine {}

        StyledTextLabel {
            text: qsTrc("palette", "Cell size")
            font.bold: true
        }

        Grid {
            width: parent.width

            columns: 2
            spacing: 16

            Repeater {
                id: repeater

                model: [
                    { title: qsTrc("palette", "Width"), value: root.cellWidth, incrementStep: 1 },
                    { title: qsTrc("palette", "Height"), value: root.cellHeight, incrementStep: 1 },
                    { title: qsTrc("palette", "Element offset"), value: root.elementOffset, measureUnit: qsTrc("palette", "sp"), incrementStep: 0.1 },
                    { title: qsTrc("palette", "Scale"), value: root.scaleFactor, incrementStep: 0.1 }
                ]

                function setValue(index, value) {
                    if (index === 0) {
                        root.cellWidth = value
                    } else if (index === 1) {
                        root.cellHeight = value
                    } else if (index === 2) {
                        root.elementOffset = value
                    } else if (index === 3) {
                        root.scaleFactor = value
                    }
                }

                Column {
                    width: parent.width / 2 - 2

                    spacing: 8

                    StyledTextLabel {
                        id: titleLabel

                        text: modelData["title"]
                    }

                    IncrementalPropertyControl {
                        id: valueField

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
            text: qsTrc("palette", "Show grid")

            checked: root.showGrid

            onClicked: {
                root.showGrid = !checked
            }
        }

        Row {
            width: parent.width
            height: childrenRect.height + 20

            spacing: 4

            FlatButton {
                text: qsTrc("palette", "Cancel")

                width: parent.width / 2

                onClicked: {
                    root.close()
                }
            }

            FlatButton {
                text: qsTrc("palette", "Ok")

                width: parent.width / 2

                onClicked: {
                    root.apply()
                    root.close()
                }
            }
        }
    }
}
