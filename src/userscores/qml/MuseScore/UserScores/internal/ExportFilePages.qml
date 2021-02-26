import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Item {
    id: exportPageStack

    property var model

    height:450

    Component.onCompleted: {
        console.info(height, "1");
    }

    Column {
        id: pdfPage

        width: parent.width
        height: parent.height

        spacing: 12

        Component.onCompleted: {
            console.info(height, "2");
        }

        ListView {
            width: parent.width
            height: parent.height

            orientation: Qt.Vertical

            model: exportPageStack.model

            Component.onCompleted: {
                console.info(height, "3");
            }

            delegate: Column {
                Layout.fillWidth: true
                Layout.preferredHeight: childrenRect.height

                Component.onCompleted: {
                    console.info(height, "4");
                }

                spacing: 6

                StyledTextLabel {
                    Layout.fillWidth: true
                    height: 30

                    text: friendlyNameRole
                    font.capitalization: Font.AllUppercase

                }

                Item {
                    id: control
                    height: 30
                    width: 150

                    Loader {
                        id: loader
                        property var val: valRole
                        anchors.fill: parent
                        sourceComponent: exportPageStack.componentByType(typeRole)
                        onLoaded: loader.item.val = loader.val
                        onValChanged: {
                            if (loader.item) {
                                loader.item.val = loader.val
                            }
                        }
                    }

                    Connections {
                        target: loader.item
                        function onChanged(newVal) {
                            exportPageStack.model.changeVal(model.index, newVal)
                        }
                    }
                }
            }
        }
    }

    function componentByType(type) {
        switch (type) {
        case "Undefined": return textComp;
        case "Bool": return boolComp;
        case "Int": return intComp;
        case "Double": return doubleComp;
        case "String": return textComp;
        case "Color": return colorComp;
        }

        return textComp;
    }

    Component {
        id: textComp

        TextInputField {
            id: textControl
            anchors.fill: parent

            property var val

            currentText: val

            signal changed(var newVal)

            onCurrentTextEdited: {
                changed(newVal)
            }
        }
    }

    Component {
        id: colorComp
        Rectangle {
            id: colorControl
            property var val
            signal changed(var newVal)
            anchors.fill: parent
            color: val

            ColorDialog {
                id: colorDialog
                title: "Please choose a color"
                onAccepted: colorControl.changed(colorDialog.color)
            }

            MouseArea {
                anchors.fill: parent
                onClicked: colorDialog.open()
            }
        }
    }

    Component {
        id: intComp

        IncrementalPropertyControl {
            iconMode: iconModeEnum.hidden

            property var val
            currentValue: val.toString()

            step: 1
            minValue: 72
            maxValue: 2400

            signal changed(var newVal)

            onValueEdited: {
                currentValue = newValue
                changed(newValue)
            }
        }
    }

    Component {
        id: doubleComp

        IncrementalPropertyControl {
            iconMode: iconModeEnum.hidden

            property var val
            currentValue: val.toString()

            step: 1
            minValue: 72
            maxValue: 2400

            signal changed(var newVal)

            onValueEdited: {
                currentValue = newValue
                changed(newValue)
            }
        }
    }

    Component {
        id: boolComp
        CheckBox {
            id: checkbox
            property var val
            signal changed(var newVal)
            anchors.fill: parent
            checked: val ? true : false
            onClicked: checkbox.changed(!checkbox.checked)
        }
    }

}
