import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

StyledPopup {
    id: root

    height: Math.max(contentColumn.implicitHeight + topPadding + bottomPadding, implicitHeight)
    width: parent.width

    implicitHeight: 290

    function load(instrument) {
        settingsModel.load(instrument)
    }

    InstrumentSettingsModel {
        id: settingsModel
    }

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        StyledTextLabel {
            text: qsTrc("instruments", "Name on main score")
        }

        TextInputField {
            currentText: settingsModel.instrumentName

            onCurrentTextEdited: {
                settingsModel.instrumentName = newTextValue
            }
        }

        StyledTextLabel {
            text: qsTrc("instruments", "Abbreviated name")
        }

        TextInputField {
            currentText: settingsModel.abbreviature

            onCurrentTextEdited: {
                settingsModel.abbreviature = newTextValue
            }
        }

        StyledTextLabel {
            text: qsTrc("instruments", "Part name")
        }

        TextInputField {
            currentText: settingsModel.partName

            onCurrentTextEdited: {
                settingsModel.partName = newTextValue
            }
        }

        SeparatorLine {
            anchors.leftMargin: -root.leftPadding + root.borderWidth
            anchors.rightMargin: -root.rightPadding + root.borderWidth
        }

        FlatButton {
            width: parent.width
            text: qsTrc("instruments", "Replace instrument")

            onClicked: {
                root.close()
                Qt.callLater(settingsModel.replaceInstrument)
            }
        }
    }
}
