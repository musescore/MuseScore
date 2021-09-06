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
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Shortcuts 1.0

Item {
    id: root

    function apply() {
        return mappingsModel.apply()
    }

    EditMidiMappingDialog {
        id: editMappingDialog

        function startEditCurrentAction() {
            editMappingDialog.startEdit(mappingsModel.currentAction())
        }

        onMapToEventRequested: {
            mappingsModel.mapCurrentActionToMidiEvent(event)
        }
    }

    MidiDeviceMappingModel {
        id: mappingsModel

        selection: view.selection
    }

    Component.onCompleted: {
        mappingsModel.load()
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 20

        CheckBox {
            text: qsTrc("shortcuts", "MIDI remote control")
            font: ui.theme.bodyBoldFont

            checked: mappingsModel.useRemoteControl

            onClicked:  {
                mappingsModel.useRemoteControl = !checked
            }
        }

        ValueList {
            id: view

            Layout.fillWidth: true
            Layout.fillHeight: true

            enabled: mappingsModel.useRemoteControl
            readOnly: true

            keyRoleName: "title"
            keyTitle: qsTrc("shortcuts", "action")
            valueRoleName: "status"
            valueTitle: qsTrc("shortcuts", "status")
            iconRoleName: "icon"
            valueEnabledRoleName: "enabled"

            model: mappingsModel

            onHandleItem: {
                editMappingDialog.startEditCurrentAction()
            }
        }

        Row {
            enabled: mappingsModel.useRemoteControl

            Layout.alignment: Qt.AlignRight

            spacing: 8

            FlatButton {
                enabled: mappingsModel.canEditAction
                text: qsTrc("shortcuts", "Assign MIDI mapping...")

                onClicked: {
                    editMappingDialog.startEditCurrentAction()
                }
            }

            FlatButton {
                width: 100

                text: qsTrc("global", "Clear")

                onClicked: {
                    mappingsModel.clearSelectedActions()
                }
            }

            FlatButton {
                width: 100

                text: qsTrc("global", "Clear all")

                onClicked: {
                    mappingsModel.clearAllActions()
                }
            }
        }
    }
}
