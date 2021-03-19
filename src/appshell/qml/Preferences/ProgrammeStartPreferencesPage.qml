import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Item {
    ProgrammeStartPreferencesModel {
        id: programmeStartModel
    }

    Column {
        anchors.fill: parent

        spacing: 20

        StyledTextLabel {
            text: qsTrc("appshell", "Programme Start")
            font: ui.theme.bodyBoldFont
        }

        RadioButtonGroup {
            spacing: 16
            orientation: Qt.Vertical

            width: parent.width

            model: programmeStartModel.startModes

            delegate: Loader {
                id: loader

                sourceComponent: modelData.canSelectScorePath ? modeRadioButtonWithPath : modeRadioButton

                onLoaded: {
                    loader.item.modelData = modelData
                    loader.item.index = model.index
                }
            }
        }

        ListView {
            spacing: 16
            interactive: false

            width: parent.width
            height: contentHeight

            model: programmeStartModel.panels

            delegate: CheckBox {
                text: modelData.title
                checked: modelData.visible

                onClicked: {
                    programmeStartModel.setPanelVisible(model.index, !checked)
                }
            }
        }
    }

    Component {
        id: modeRadioButton

        RoundedRadioButton {
            property var modelData
            property int index: -1

            width: 220
            padding: 0
            spacing: 6

            checked: modelData.checked

            onClicked: {
                programmeStartModel.setCurrentStartMode(index, "")
            }

            StyledTextLabel {
                text: modelData.title
                horizontalAlignment: Qt.AlignLeft
            }
        }
    }

    Component {
        id: modeRadioButtonWithPath

        Row {
            property var modelData
            property int index: -1

            spacing: 0

            Loader {
                id: radioButtonLoader

                sourceComponent: modeRadioButton
            }

            onModelDataChanged: {
                radioButtonLoader.item.modelData = modelData
                radioButtonLoader.item.index = index
            }

            FilePicker {
                width: 240

                path: modelData.scorePath
            }
        }
    }
}
