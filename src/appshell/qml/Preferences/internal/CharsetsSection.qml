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

        ComboBoxWithTitle {
            title: qsTrc("appshell", "Guitar Pro import character set:")
            titleWidth: 220

            currentIndex: control.indexOfValue(preferencesModel.currentGuitarProCharset)
            model: preferencesModel.charsets()

            onValueEdited: {
                preferencesModel.currentGuitarProCharset = newValue
            }
        }

        ComboBoxWithTitle {
            title: qsTrc("appshell", "Overture import character set:")
            titleWidth: 220

            currentIndex: control.indexOfValue(preferencesModel.currentOvertuneCharset)
            model: preferencesModel.charsets()

            onValueEdited: {
                preferencesModel.currentOvertuneCharset = newValue
            }
        }
    }
}
