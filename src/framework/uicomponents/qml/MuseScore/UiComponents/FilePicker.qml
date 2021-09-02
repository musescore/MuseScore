/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

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

    property NavigationPanel navigation: null
    property int navigationRowOrder: 0
    property int navigationColumnOrderStart: 0

    property string pathFieldTitle: qsTrc("uicomponents", "Current path:")

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

            navigation.name: "PathFieldBox"
            navigation.panel: root.navigation
            navigation.row: root.navigationRowOrder
            navigation.enabled: root.visible && root.enabled
            navigation.column: root.navigationColumnOrderStart
            navigation.accessible.name: root.pathFieldTitle + " " + pathField.currentText

            onCurrentTextEdited: {
                root.pathEdited(newTextValue)
            }
        }

        FlatButton {
            Layout.alignment: Qt.AlignVCenter
            icon: IconCode.OPEN_FILE

            navigation.name: "FilePickerButton"
            navigation.panel: root.navigation
            navigation.row: root.navigationRowOrder
            navigation.enabled: root.visible && root.enabled
            navigation.column: root.navigationColumnOrderStart + 1
            accessible.name: root.pickerType === FilePicker.PickerType.File ? qsTrc("uicomponents", "File choose")
                                                                            : qsTrc("uicomponents", "Directory choose")

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
