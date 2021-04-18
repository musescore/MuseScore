import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

PreferencesPage {
    id: root

    contentHeight: content.height

    FoldersPreferencesModel {
        id: foldersPreferencesModel
    }

    Component.onCompleted: {
        foldersPreferencesModel.load()
    }

    Column {
        id: content

        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 18

        StyledTextLabel {
            text: qsTrc("appshell", "Folders")
            font: ui.theme.bodyBoldFont
        }

        ListView {
            anchors.left: parent.left
            anchors.right: parent.right

            height: contentHeight

            spacing: 4

            model: foldersPreferencesModel

            delegate: RowLayout {
                width: ListView.view.width
                height: 30

                spacing: 20

                StyledTextLabel {
                    Layout.alignment: Qt.AlignLeft

                    text: model.title + ":"
                }

                FilePicker {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 380

                    pickerType: FilePicker.PickerType.Directory
                    dialogTitle: qsTrc("appshell", "Choose %1 Folder").arg(model.title)
                    dir: model.path

                    path: model.path

                    onPathEdited: {
                        model.path = newPath
                    }
                }
            }
        }
    }
}
