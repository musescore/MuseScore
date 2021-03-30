import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

PreferencesPage {
    id: root

    Component.onCompleted: {
        preferencesModel.load()
    }

    GeneralPreferencesModel {
        id: preferencesModel
    }

    Column {
        anchors.fill: parent
        spacing: 24

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            StyledTextLabel {
                text: qsTrc("appshell", "Languages")
                font: ui.theme.bodyBoldFont
            }

            Row {
                spacing: 12

                StyledComboBox {
                    width: 208

                    property var currValue

                    textRoleName: "name"
                    valueRoleName: "code"

                    model: preferencesModel.languages

                    currentIndex: indexOfValue(preferencesModel.currentLanguageCode)

                    onValueChanged: {
                        preferencesModel.currentLanguageCode = value
                    }
                }

                FlatButton {
                    text: qsTrc("appshell", "Update Translations")

                    onClicked: {
                        root.hideRequested()
                        preferencesModel.openUpdateTranslationsPage()
                    }
                }
            }
        }

        SeparatorLine { }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            StyledTextLabel {
                text: qsTrc("appshell", "Telemetry")
                font: ui.theme.bodyBoldFont
            }

            CheckBox {
                width: 216
                text: qsTrc("appshell", "Send anonymous telemetry data to MuseScore")

                checked: preferencesModel.isTelemetryAllowed

                onClicked: {
                    preferencesModel.isTelemetryAllowed = !preferencesModel.isTelemetryAllowed
                }
            }
        }

        SeparatorLine { }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            StyledTextLabel {
                text: qsTrc("appshell", "Auto Save")
                font: ui.theme.bodyBoldFont
            }

            Row {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 0

                CheckBox {
                    width: 216
                    text: qsTrc("appshell", "Auto save every:")

                    checked: preferencesModel.isAutoSave

                    onClicked: {
                        preferencesModel.isAutoSave = !preferencesModel.isAutoSave
                    }
                }

                IncrementalPropertyControl {
                    width: 96
                    iconMode: iconModeEnum.hidden

                    enabled: preferencesModel.isAutoSave

                    currentValue: preferencesModel.autoSavePeriod
                    minValue: 1
                    maxValue: 100
                    step: 1

                    measureUnitsSymbol: qsTrc("appshell", "min")

                    onValueEdited: {
                        preferencesModel.autoSavePeriod = newValue
                    }
                }
            }
        }

        SeparatorLine { }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            StyledTextLabel {
                text: qsTrc("appshell", "OSC Remote Control")
                font: ui.theme.bodyBoldFont
            }

            Row {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 0

                CheckBox {
                    width: 216
                    text: qsTrc("appshell", "Port number:")

                    checked: preferencesModel.isOSCRemoteControl

                    onClicked: {
                        preferencesModel.isOSCRemoteControl = !preferencesModel.isOSCRemoteControl
                    }
                }

                IncrementalPropertyControl {
                    width: 96
                    iconMode: iconModeEnum.hidden

                    enabled: preferencesModel.isOSCRemoteControl

                    currentValue: preferencesModel.oscPort
                    minValue: 1
                    maxValue: 65535
                    step: 1

                    onValueEdited: {
                        preferencesModel.oscPort = newValue
                    }
                }
            }
        }
    }
}
