/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.MuseSounds

Rectangle {
    color: ui.theme.backgroundSecondaryColor

    MuseSoundsDevToolsModel {
        id: model
    }

    Column {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12

        spacing: 12

        CheckBox {
            text: "Enable test mode"
            checked: model.enableTestMode
            onClicked: {
                model.enableTestMode = !checked
            }
        }

        SeparatorLine {}

        StyledTextLabel {
            text: "Update Data"
            font: ui.theme.headerBoldFont
        }

        Row {
            width: parent.width
            spacing: 12

            Column {
                width: (parent.width - 24)/2
                spacing: 12

                RowLayout {
                    width: parent.width
                    spacing: 12

                    StyledTextLabel {
                        Layout.preferredWidth: 120
                        text: "Version"
                        horizontalAlignment: Text.AlignLeft
                    }

                    IncrementalPropertyControl {
                        Layout.preferredWidth: 100
                        Layout.alignment: Qt.AlignLeft

                        currentValue: model.version
                        step: 1
                        decimals: 0
                        minValue: 0
                        maxValue: 9999

                        onValueEdited: function(newValue){
                            model.version = newValue
                        }
                    }
                }

                RowLayout {
                    width: parent.width
                    spacing: 12

                    StyledTextLabel {
                        Layout.preferredWidth: 120
                        text: "Hero image"
                        horizontalAlignment: Text.AlignLeft
                    }

                    FilePicker {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 200
                        pickerType: FilePicker.PickerType.File
                        dialogTitle: "Choose hero image"
                        filter: "Images (*.png *.jpg *.jpeg *.gif *.webp)"
                        path: model.heroImagePath
                        onPathEdited: function(newPath){
                            model.heroImagePath = newPath
                        }
                    }
                }

                RowLayout {
                    width: parent.width
                    spacing: 12

                    StyledTextLabel {
                        Layout.preferredWidth: 120
                        Layout.topMargin: 4
                        Layout.alignment: Qt.AlignTop
                        text: "Update notes"
                        horizontalAlignment: Text.AlignLeft
                    }

                    TextInputArea {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 200
                        Layout.minimumHeight: 80
                        currentText: model.updateNotes
                        onTextChanged: function(newValue){
                            model.updateNotes = newValue
                        }
                    }
                }

                RowLayout {
                    width: parent.width
                    spacing: 12

                    StyledTextLabel {
                        Layout.preferredWidth: 120
                        text: "CTA link"
                        horizontalAlignment: Text.AlignLeft
                    }

                    TextInputField {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 200
                        currentText: model.ctaLink
                        onTextChanged: function(newValue){
                            model.ctaLink = newValue
                        }
                    }
                }
            }

            ColumnLayout {
                width: (parent.width - 24)/2
                height: parent.height
                spacing: 12

                StyledTextLabel {
                    text: "Result"
                    horizontalAlignment: Text.AlignLeft
                }

                TextInputArea {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentText: model.currentUpdateData

                    onTextEditingFinished: function(newValue){
                        model.currentUpdateData = newValue
                    }
                }
            }
        }

        FlatButton {
            text: "Apply"
            onClicked: model.applyUpdateData()
        }

        SeparatorLine {}

        StyledTextLabel {
            text: "Show Result"
            font: ui.theme.headerBoldFont
        }

        RowLayout {
            width: (parent.width - 24)/2
            spacing: 12

            StyledTextLabel {
                Layout.preferredWidth: 120
                text: "Language"
                horizontalAlignment: Text.AlignLeft
            }

            StyledDropdown {
                Layout.preferredWidth: 150
                model: model.availableLanguages
                textRole: "modelData"
                valueRole: "modelData"
                currentIndex: model.availableLanguages.indexOf(model.selectedLanguage)
                onActivated: function(index, value) {
                    model.selectedLanguage = value
                }
            }
        }

        FlatButton {
            text: "Open Dialog"
            onClicked: model.openUpdateDialog()
        }
    }
}
