import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

QmlDialog {
    id: root

    property string paletteName: ""
    property bool showGrid: false
    property int cellWidth: 0
    property int cellHeight: 0
    property real elementOffset: 0
    property real scaleFactor: 0

    width: 280
    height: 370

    title: qsTrc("palette", "Palette Properties")

    Rectangle {
        color: ui.theme.backgroundPrimaryColor

        Column {
            anchors.fill: parent

            readonly property int margins: 12
            anchors.margins: margins

            spacing: 12

            StyledTextLabel {
                text: qsTrc("palette", "Name")
                font.bold: true
            }

            TextInputField {
                currentText: root.paletteName

                onCurrentTextEdited: {
                    root.paletteName = newTextValue
                }
            }

            SeparatorLine { anchors.margins: -margins }

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
                text: qsTrc("palette", "Show grid")

                checked: root.showGrid

                onClicked: {
                    root.showGrid = !checked
                }
            }

            Item { height: 1; width: parent.width }

            Row {
                width: parent.width
                height: childrenRect.height + 20

                spacing: 4

                FlatButton {
                    text: qsTrc("palette", "Cancel")

                    width: parent.width / 2

                    onClicked: {
                        root.ret = { errcode: 0}
                        root.hide()
                    }
                }

                FlatButton {
                    text: qsTrc("palette", "Ok")

                    width: parent.width / 2

                    onClicked: {
                        root.ret = { errcode: 0}
                        root.hide()
                    }
                }
            }
        }
    }
}
