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
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Midi 1.0

ColumnLayout {
    anchors.fill: parent
    anchors.margins: 12
    spacing: 12

    MidiPortDevModel {
        id: midiModel
    }

    Column {
        Layout.fillWidth: true
        spacing: 12

        StyledTextLabel {
            width: parent.width
            horizontalAlignment: Text.AlignLeft
            font: ui.theme.bodyBoldFont
            text: "MIDI output devices:"
        }

        StyledListView {
            width: parent.width
            height: contentHeight

            model: midiModel.outputDevices

            delegate: RowLayout {
                width: ListView.view.width
                height: 40
                spacing: 12

                StyledTextLabel {
                    Layout.minimumWidth: 240
                    horizontalAlignment: Text.AlignLeft
                    text: modelData.id + " " + modelData.name
                }

                FlatButton {
                    text: modelData.action
                    onClicked: {
                        midiModel.outputDeviceAction(modelData.id, modelData.action)
                    }
                }

                StyledTextLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft
                    text: modelData.error
                }
            }
        }
    }

    Column {
        Layout.fillWidth: true
        spacing: 12

        StyledTextLabel {
            width: parent.width
            horizontalAlignment: Text.AlignLeft
            font: ui.theme.bodyBoldFont
            text: "MIDI input devices:"
        }

        StyledListView {
            width: parent.width
            height: contentHeight

            model: midiModel.inputDevices

            delegate: RowLayout {
                width: ListView.view.width
                height: 40
                spacing: 12

                StyledTextLabel {
                    Layout.minimumWidth: 240
                    horizontalAlignment: Text.AlignLeft
                    text: modelData.id + " " + modelData.name
                }

                FlatButton {
                    text: modelData.action
                    onClicked: {
                        midiModel.inputDeviceAction(modelData.id, modelData.action)
                    }
                }

                StyledTextLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft
                    text: modelData.error
                }
            }
        }
    }

    FlatButton {
        text: "Generate MIDI2.0"
        onClicked: midiModel.generateMIDI20();
    }

    StyledListView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        model: midiModel.inputEvents

        delegate: StyledTextLabel {
            width: ListView.view.width
            height: 20
            horizontalAlignment: Text.AlignLeft
            text: modelData
        }
    }
}
