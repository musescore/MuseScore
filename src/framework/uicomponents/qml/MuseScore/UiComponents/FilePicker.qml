import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    enum PickerType {
        File,
        Directory,
        MultipleDirectories
    }
    property int pickerType: FilePicker.PickerType.File

    property alias path: pathField.currentText

    property alias dialogTitle: filePickerModel.title
    property alias filter: filePickerModel.filter
    property alias dir: filePickerModel.dir

    signal pathEdited(var newPath)

    height: 30

    FilePickerModel {
        id: filePickerModel
    }

    RowLayout {
        anchors.fill: parent
        spacing: 8

        TextInputField {
            id: pathField
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            onCurrentTextEdited: {
                root.pathEdited(newTextValue)
            }
        }

        FlatButton {
            Layout.alignment: Qt.AlignVCenter
            icon: IconCode.OPEN_FILE

            onClicked: {
                var selectedPath

                switch (pickerType) {
                case FilePicker.PickerType.File:
                    selectedPath = filePickerModel.selectFile()
                    break
                case FilePicker.PickerType.Directory:
                    selectedPath = filePickerModel.selectDirectory()
                    break
                case FilePicker.PickerType.MultipleDirectories:
                    selectedPath = filePickerModel.selectMultipleDirectories(root.path)
                    break
                }

                if (!Boolean(selectedPath)) {
                    return
                }

                root.pathEdited(selectedPath)
            }
        }
    }
}
