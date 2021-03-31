import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Column {
    spacing: 18

    property var preferencesModel: null

    StyledTextLabel {
        text: qsTrc("appshell", "Character Set Used When Importing Binary Files")
        font: ui.theme.bodyBoldFont
    }

    Column {
        spacing: 12

        Row {
            spacing: 12

            StyledTextLabel {
                width: 208
                anchors.verticalCenter: parent.verticalCenter
                text: qsTrc("appshell", "Guitar Pro import character set:")
                horizontalAlignment: Text.AlignLeft
            }

            StyledComboBox {
                implicitWidth: 208

                currentIndex: indexOfValue(preferencesModel.currentGuitarProCharset)
                model: preferencesModel.charsets()

                onValueChanged: {
                    preferencesModel.currentGuitarProCharset = value
                }
            }
        }

        Row {
            spacing: 12

            StyledTextLabel {
                width: 208
                anchors.verticalCenter: parent.verticalCenter
                text: qsTrc("appshell", "Overture import character set:")
                horizontalAlignment: Text.AlignLeft
            }

            StyledComboBox {
                implicitWidth: 208

                currentIndex: indexOfValue(preferencesModel.currentOvertuneCharset)
                model: preferencesModel.charsets()

                onValueChanged: {
                    preferencesModel.currentOvertuneCharset = value
                }
            }
        }
    }
}
