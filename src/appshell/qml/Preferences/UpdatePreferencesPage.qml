import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Item {
    UpdatePreferencesModel {
        id: updateModel
    }

    Column {
        anchors.fill: parent

        spacing: 18

        StyledTextLabel {
            text: qsTrc("appshell", "Automatic Update Check")
            font: ui.theme.bodyBoldFont
        }

        CheckBox {
            text: qsTrc("appshell", "Check for new version of Musescore")

            visible: updateModel.isAppUpdatable()
            checked: updateModel.needCheckForNewAppVersion

            onClicked: {
                updateModel.needCheckForNewAppVersion = !checked
            }
        }

        CheckBox {
            width: 200

            text: qsTrc("appshell", "Check for new version of MuseScore extensions")

            checked: updateModel.needCheckForNewExtensionsVersion

            onClicked: {
                updateModel.needCheckForNewExtensionsVersion = !checked
            }
        }
    }
}
