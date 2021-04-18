import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Column {
    spacing: 18

    property var preferencesModel: null

    StyledTextLabel {
        text: qsTrc("appshell", "MusicXML")
        font: ui.theme.bodyBoldFont
    }

    Column {
        spacing: 12

        CheckBox {
            width: 208
            text: qsTrc("appshell", "Import layout")
            checked: preferencesModel.importLayout

            onClicked: {
                preferencesModel.importLayout = !preferencesModel.importLayout
            }
        }

        CheckBox {
            width: 208
            text: qsTrc("appshell", "Import system and page breaks")
            checked: preferencesModel.importBreaks

            onClicked: {
                preferencesModel.importBreaks = !preferencesModel.importBreaks
            }
        }

        CheckBox {
            width: 208
            text: qsTrc("appshell", "Apply default typeface (Edwin) to imported scores")
            checked: preferencesModel.needUseDefaultFont

            onClicked: {
                preferencesModel.needUseDefaultFont = !preferencesModel.needUseDefaultFont
            }
        }
    }
}
