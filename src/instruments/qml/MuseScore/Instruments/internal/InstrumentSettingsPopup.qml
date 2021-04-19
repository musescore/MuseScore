import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

StyledPopupView {
    id: root

    contentHeight: contentColumn.childrenRect.height
    contentWidth: 240

    function load(instrument) {
        settingsModel.load(instrument)
    }

    InstrumentSettingsModel {
        id: settingsModel
    }

    Column {
        id: contentColumn

        anchors.fill: parent
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
            keynav.subsection: root.keynav
            text: qsTrc("instruments", "Replace instrument")

            onClicked: {
                root.close()
                Qt.callLater(settingsModel.replaceInstrument)
            }
        }
    }
}
