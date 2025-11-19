/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import Muse.Ui 1.0
import Muse.UiComponents

Item {
    id: root

    enum PickerType {
        File,
        Directory,
        MultipleDirectories,
        Any
    }
    property int pickerType: FilePicker.PickerType.File

    property alias path: pathField.currentText

    property alias dialogTitle: filePickerModel.title
    property alias filter: filePickerModel.filter
    property alias dir: filePickerModel.dir

    property int buttonType: FlatButton.IconOnly
    property int orientation: Qt.Vertical

    property NavigationPanel navigation: null
    property int navigationRowOrderStart: 0
    property int navigationColumnOrderStart: 0

    property string pathFieldTitle: qsTrc("ui", "Current path:")
    property alias pathFieldWidth: pathField.implicitWidth

    property alias buttonWidth: button.implicitWidth

    property alias spacing: row.spacing

    signal pathEdited(var newPath)

    implicitWidth: row.implicitWidth
    implicitHeight: row.implicitHeight

    FilePickerModel {
        id: filePickerModel
    }

    Item {
        id: prv

        property bool isNavigationBoth: root.navigation && root.navigation.direction === NavigationPanel.Both

        states: [
            State {
                when: prv.isNavigationBoth
                PropertyChanges {
                    pathField.navigation.row: root.navigationRowOrderStart
                    pathField.navigation.column: root.navigationColumnOrderStart

                    button.navigation.row: root.navigationRowOrderStart
                    button.navigation.column: root.navigationColumnOrderStart + 1
                }
            },
            State {
                when: !prv.isNavigationBoth
                PropertyChanges {
                    pathField.navigation.order: root.navigationRowOrderStart
                    button.navigation.order: root.navigationRowOrderStart + 1
                }
            }
        ]
    }

    RowLayout {
        id: row
        anchors.fill: parent
        spacing: 12

        TextInputField {
            id: pathField

            Layout.fillWidth: true
            Layout.minimumWidth: implicitWidth
            Layout.alignment: Qt.AlignVCenter

            implicitWidth: 0

            navigation.name: "PathFieldBox"
            navigation.panel: root.navigation
            navigation.enabled: root.visible && root.enabled
            navigation.accessible.name: root.pathFieldTitle + " " + pathField.currentText

            onTextEditingFinished: function(newTextValue) {
                root.pathEdited(newTextValue)
            }
        }

        FlatButton {
            id: button
            Layout.alignment: Qt.AlignVCenter
            icon: IconCode.OPEN_FILE

            text: qsTrc("ui", "Browse")
            buttonType: root.buttonType
            orientation: root.orientation

            navigation.name: "FilePickerButton"
            navigation.panel: root.navigation
            navigation.enabled: root.visible && root.enabled
            accessible.name: root.pickerType === FilePicker.PickerType.File ? qsTrc("ui", "Choose file")
                                                                            : qsTrc("ui", "Choose directory")

            onClicked: {
                switch (root.pickerType) {
                case FilePicker.PickerType.File: {
                    var selectedFile = filePickerModel.selectFile()
                    if (Boolean(selectedFile)) {
                        root.pathEdited(selectedFile)
                    }

                    break
                }
                case FilePicker.PickerType.Directory: {
                    var selectedDirectory = filePickerModel.selectDirectory()
                    if (Boolean(selectedDirectory)) {
                        root.pathEdited(selectedDirectory)
                    }

                    break
                }
                case FilePicker.PickerType.MultipleDirectories:{
                    var selectedDirectories = filePickerModel.selectMultipleDirectories(root.path)
                    root.pathEdited(selectedDirectories)
                    break
                }
                case FilePicker.PickerType.Any:{
                    var selectedAny = filePickerModel.selectAny()
                    if (Boolean(selectedAny)) {
                        root.pathEdited(selectedAny)
                    }

                    break
                }
                }
            }
        }
    }
}
