import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2

import MuseScore.UserScores 1.0
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

ColumnLayout {
    id: root
    required property ExportScoreModel scoresModel
    required property ExportScoreSuffixModel suffixModel
    required property ExportScoreSettingsModel settingsModel

    spacing: 12
    property var firstColumnWidth: 66

    RowLayout {
        id: exportSuffixSelection
        spacing: 12

        StyledTextLabel {
            Layout.preferredWidth: firstColumnWidth
            horizontalAlignment: Text.AlignLeft
            text: qsTrc("userscores", "Format:")
        }

        StyledComboBox {
            Layout.fillWidth: true

            textRoleName: "suffix"
            valueRoleName: "value"

            model: suffixModel

            onValueChanged: {
                currentIndex = indexOfValue(value)
                scoresModel.setExportSuffix(valueFromModel(currentIndex, "suffix"));
                settingsModel.load(valueFromModel(currentIndex, "suffix"));
            }

            Component.onCompleted: {
                value = suffixModel.getDefaultRow()
            }
        }
    }

    Repeater {
        model: settingsModel

        delegate: Loader {
            Layout.fillWidth: true

            sourceComponent: {
                switch (type) {
                case "Bool": return checkboxComp
                case "String": return textComp
                case "Color": return colorComp
                case "Int":
                case "Double":
                case "NumberSpinner": return spinboxComp
                case "ComboBox": return comboBoxComp
                case "RadioButtonGroup": return radioButtonGroupComp
                default: console.error("Unknown type", type); return textComp
                }
            }

            property string type: typeRole
            property var val: valRole
            property string label: labelRole
            property var info: infoRole

            Connections {
                target: item
                function onChanged(newValue) {
                    settingsModel.setValue(index, newValue)
                }
            }
        }
    }

    Component {
        id: checkboxComp

        CheckBox {
            text: label
            signal changed(var newValue)
            checked: val
            onClicked: changed(!checked)
        }
    }

    Component {
        id: spinboxComp

        Row {
            id: spinboxCompRow
            spacing: 12

            signal changed(var newValue)

            StyledTextLabel {
                width: firstColumnWidth
                text: label
                horizontalAlignment: Text.AlignLeft
                anchors.verticalCenter: parent.verticalCenter
            }

            IncrementalPropertyControl {
                width: 80
                iconMode: iconModeEnum.hidden

                currentValue: val

                // When type is just "int" or "double", info won't have values
                minValue: type === "NumberSpinner" ? info.minValue : 0
                maxValue: type === "NumberSpinner" ? info.maxValue : 999
                step:     type === "NumberSpinner" ? info.step     : 1
                decimals: {
                    if (type === "NumberSpinner") {
                        return info.decimals
                    } else if (type === "Int") {
                        return 0
                    } else if (type === "Double") {
                        return 2
                    }
                    return 0
                }
                measureUnitsSymbol: type === "NumberSpinner" ? info.measureUnitsSymbol : ""

                onValueEdited: {
                    spinboxCompRow.changed(newValue)
                }
            }
        }
    }

    Component {
        id: textComp

        RowLayout {
            id: textCompRow
            spacing: 12

            signal changed(var newValue)

            StyledTextLabel {
                Layout.preferredWidth: firstColumnWidth
                text: label
                horizontalAlignment: Text.AlignLeft
            }

            TextInputField {
                Layout.fillWidth: true
                currentText: val

                onCurrentTextEdited: {
                    textCompRow.changed(newTextValue)
                }
            }
        }
    }

    Component {
        id: colorComp

        Rectangle {
            id: colorControl
            color: val

            signal changed(var newValue)

            Component.onCompleted: {
                console.warn("colorComp in Export Dialog has not been tested yet.
Please review its functionality, make changes if necessary and remove this message.")
            }

            ColorDialog {
                id: colorDialog
                title: qsTrc("userscores", "Please choose a color")
                onAccepted: colorControl.changed(colorDialog.color)
            }

            MouseArea {
                anchors.fill: parent
                onClicked: colorDialog.open()
            }
        }
    }

    Component {
        id: comboBoxComp

        RowLayout {
            id: comboBoxCompRow
            spacing: 12

            signal changed(var newValue)

            StyledTextLabel {
                Layout.preferredWidth: firstColumnWidth
                text: label
                horizontalAlignment: Text.AlignLeft
            }

            StyledComboBox {
                Layout.fillWidth: true

                model: info.model

                textRoleName: "textRole"
                valueRoleName: "valueRole"
                currentIndex: indexOfValue(val)
                onValueChanged: {
                    comboBoxCompRow.changed(newValue)
                }
            }
        }
    }

    Component {
        id: radioButtonGroupComp

        ColumnLayout {
            id: radioButtonGroupCompRow
            spacing: 12

            signal changed(var newValue)

            StyledTextLabel {
                Layout.fillWidth: true
                text: label
                horizontalAlignment: Text.AlignLeft
            }

            RadioButtonGroup {
                id: radioButtonGroup
                Layout.fillWidth: true
                orientation: Qt.Vertical
                spacing: 12

                model: info.model

                delegate: RoundedRadioButton {
                    text: textRole
                    ButtonGroup.group: radioButtonGroup.radioButtonGroup

                    checked: valueRole === val

                    onToggled: {
                        radioButtonGroupCompRow.changed(valueRole)
                    }
                }
            }
        }
    }
}
