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
import QtQuick.Dialogs 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Shortcuts 1.0

Dialog {
    id: root

    signal mapToEventRequested(var event)

    title: qsTrc("shortcuts", "MIDI remote control")

    height: 220
    width: 538

    standardButtons: Dialog.NoButton

    function startEdit(action) {
        model.load(action.mappedType, action.mappedValue)
        open()

        actionNameLabel.text = action.title
        actionIconLabel.iconCode = action.icon
    }

    EditMidiMappingModel {
        id: model
    }

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        Column {
            anchors.fill: parent
            anchors.margins: 8

            spacing: 24

            Row {
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: 8

                StyledIconLabel {
                    id: actionIconLabel
                }

                StyledTextLabel {
                    id: actionNameLabel

                    font: ui.theme.bodyBoldFont
                }
            }

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                text: qsTrc("shortcuts", "Press a key or adjust a control on your MIDI device to\nassign it to this function.")
            }

            RowLayout {
                width: parent.width

                spacing: 10

                StyledTextLabel {
                    text: qsTrc("shortcuts", "MIDI mapping:")
                }

                TextInputField {
                    id: mappingField

                    Layout.fillWidth: true

                    readOnly: true

                    currentText: model.mappingTitle
                    hint: qsTrc("global", "Waiting…")
                }
            }

            Row {
                anchors.right: parent.right

                spacing: 8

                FlatButton {
                    width: 100

                    text: qsTrc("global", "Add")
                    enabled: mappingField.hasText

                    onClicked: {
                        root.mapToEventRequested(model.inputedEvent())
                        root.close()
                    }
                }

                FlatButton {
                    width: 100

                    text: qsTrc("global", "Cancel")

                    onClicked: {
                        root.reject()
                    }
                }
            }
        }
    }
}
