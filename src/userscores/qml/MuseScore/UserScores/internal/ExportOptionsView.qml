import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2

import MuseScore.UserScores 1.0
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

ColumnLayout {
    required property ExportScoreModel scoresModel
    required property ExportScoreSuffixModel suffixModel
    required property ExportScoreSettingsModel settingsModel

    spacing: 12

    Column {
        spacing: 6
        Layout.fillWidth: true

        StyledTextLabel {
            Layout.fillWidth: true
            text: qsTrc("userscores", "Export To:")
            font.capitalization: Font.AllUppercase
        }

        Row {
            spacing: 6
            width: parent.width

            TextInputField {
                id: fileExportPathInput

                // Would be better to use `RowLayout` and `Layout.fillWidth: true`,
                // but causes crash like https://bugreports.qt.io/browse/QTBUG-77337
                width: parent.width - parent.spacing - browsePathButton.width

                currentText: scoresModel.getExportPath()

                onCurrentTextEdited: {
                    scoresModel.setExportPath(newTextValue);
                }
            }

            FlatButton {
                id: browsePathButton
                icon: IconCode.NEW_FILE

                height: 30

                onClicked: {
                    fileExportPathInput.currentText = scoresModel.chooseExportPath();
                }
            }
        }
    }

    Column {
        id: exportSuffixSelection

        Layout.fillWidth: true

        spacing: 6

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.topMargin: 24

            text: qsTrc("userscores", "Export As:")
            font.capitalization: Font.AllUppercase
        }

        StyledComboBox {
            width: parent.width

            textRoleName: "suffix"
            valueRoleName: "value"

            model: suffixModel

            onValueChanged: {
                currentIndex = indexOfValue(value)
                scoresModel.setExportSuffix(valueFromModel(currentIndex, "suffix"));
                fileExportPathInput.currentText = scoresModel.exportPath();

                settingsModel.changeType(valueFromModel(currentIndex, "suffix"));
            }

            Component.onCompleted: {
                value = suffixModel.getDefaultRow()
            }
        }
    }

    Repeater {
        model: settingsModel

        delegate: Column {
            spacing: 6
            height: textLabel.height + spacing + control.height

            StyledTextLabel {
                id: textLabel
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
                    sourceComponent: componentByType(typeRole)
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
                        settingsModel.changeVal(model.index, newVal)
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
            onClicked: changed(!checked)
        }
    }

    Item { // spacer
        Layout.fillHeight: true
        Layout.topMargin: -parent.spacing
    }
}
