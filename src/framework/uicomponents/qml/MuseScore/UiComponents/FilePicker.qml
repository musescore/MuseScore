/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    enum PickerType {
        File,
        Directory
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
                if (pickerType === FilePicker.PickerType.File) {
                    selectedPath = filePickerModel.selectFile()
                } else {
                    selectedPath = filePickerModel.selectDirectory()
                }

                if (!Boolean(selectedPath)) {
                    return
                }

                root.pathEdited(selectedPath)
            }
        }
    }
}
