import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Column {
    spacing: 18

    property var preferencesModel: null

    StyledTextLabel {
        text: qsTrc("appshell", "Style Used for Import")
        font: ui.theme.bodyBoldFont
    }

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 12

        RoundedRadioButton {
            anchors.left: parent.left
            anchors.right: parent.right

            checked: preferencesModel.styleFileImportPath === ""

            StyledTextLabel {
                text: qsTrc("appshell", "Built-in style")
                horizontalAlignment: Text.AlignLeft
            }

            onToggled: {
                preferencesModel.styleFileImportPath = ""
            }
        }

        RoundedRadioButton {
            anchors.left: parent.left
            anchors.right: parent.right

            checked: preferencesModel.styleFileImportPath !== ""

            onToggled: {
                preferencesModel.styleFileImportPath = ""
            }

            Item {
                StyledTextLabel {
                    id: title

                    width: 193
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTrc("appshell", "Use style file:")
                    horizontalAlignment: Text.AlignLeft
                }

                FilePicker {
                    anchors.left: title.right
                    width: 246
                    anchors.verticalCenter: parent.verticalCenter

                    dialogTitle: preferencesModel.styleChooseTitle()
                    filter: preferencesModel.stylePathFilter()
                    dir: preferencesModel.fileDirectory(preferencesModel.styleFileImportPath)

                    path: preferencesModel.styleFileImportPath

                    onPathEdited: {
                        preferencesModel.styleFileImportPath = newPath
                    }
                }
            }
        }
    }
}
