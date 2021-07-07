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
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

PreferencesPage {
    id: root

    Component.onCompleted: {
        preferencesModel.load()
    }

    GeneralPreferencesModel {
        id: preferencesModel
    }

    Column {
        anchors.fill: parent
        spacing: 24

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            StyledTextLabel {
                text: qsTrc("appshell", "Languages")
                font: ui.theme.bodyBoldFont
            }

            Row {
                spacing: 12

                Dropdown {
                    id: dropdown

                    width: 208

                    textRole: "name"
                    valueRole: "code"

                    model: preferencesModel.languages

                    currentIndex: dropdown.indexOfValue(preferencesModel.currentLanguageCode)

                    onCurrentValueChanged: {
                        preferencesModel.currentLanguageCode = dropdown.currentValue
                    }
                }

                FlatButton {
                    text: qsTrc("appshell", "Update Translations")

                    onClicked: {
                        root.hideRequested()
                        preferencesModel.openUpdateTranslationsPage()
                    }
                }
            }
        }

        //SeparatorLine { }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            visible: false

            StyledTextLabel {
                text: qsTrc("appshell", "Telemetry")
                font: ui.theme.bodyBoldFont
            }

            CheckBox {
                width: 216
                text: qsTrc("appshell", "Send anonymous telemetry data to MuseScore")

                checked: preferencesModel.isTelemetryAllowed

                onClicked: {
                    preferencesModel.isTelemetryAllowed = !preferencesModel.isTelemetryAllowed
                }
            }
        }

        SeparatorLine { }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            StyledTextLabel {
                text: qsTrc("appshell", "Auto Save")
                font: ui.theme.bodyBoldFont
            }

            Row {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 0

                CheckBox {
                    width: 216
                    text: qsTrc("appshell", "Auto save every:")

                    checked: preferencesModel.isAutoSave

                    onClicked: {
                        preferencesModel.isAutoSave = !preferencesModel.isAutoSave
                    }
                }

                IncrementalPropertyControl {
                    width: 96
                    iconMode: iconModeEnum.hidden

                    enabled: preferencesModel.isAutoSave

                    currentValue: preferencesModel.autoSavePeriod
                    minValue: 1
                    maxValue: 100
                    step: 1

                    measureUnitsSymbol: qsTrc("appshell", "min")

                    onValueEdited: {
                        preferencesModel.autoSavePeriod = newValue
                    }
                }
            }
        }

        SeparatorLine { }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            StyledTextLabel {
                text: qsTrc("appshell", "OSC Remote Control")
                font: ui.theme.bodyBoldFont
            }

            Row {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 0

                CheckBox {
                    width: 216
                    text: qsTrc("appshell", "Port number:")

                    checked: preferencesModel.isOSCRemoteControl

                    onClicked: {
                        preferencesModel.isOSCRemoteControl = !preferencesModel.isOSCRemoteControl
                    }
                }

                IncrementalPropertyControl {
                    width: 96
                    iconMode: iconModeEnum.hidden

                    enabled: preferencesModel.isOSCRemoteControl

                    currentValue: preferencesModel.oscPort
                    minValue: 1
                    maxValue: 65535
                    step: 1

                    onValueEdited: {
                        preferencesModel.oscPort = newValue
                    }
                }
            }
        }
    }
}
