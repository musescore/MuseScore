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
import QtQuick 2.7
import QtQuick.Controls 2.2
import MuseScore.UiComponents 1.0
import MuseScore.Midi 1.0

Item {

    MidiPortDevModel {
        id: midiModel

        onOutputDevicesChanged: {
            outputDevicesRepeater.model = 0
            outputDevicesRepeater.model = midiModel.outputDevices()
        }

        onInputDevicesChanged: {
            inputDevicesRepeater.model = 0
            inputDevicesRepeater.model = midiModel.inputDevices()
        }

        onInputEventsChanged: {
            inputEventsRepeator.model = 0
            inputEventsRepeator.model = midiModel.inputEvents()
        }
    }

    Flickable {
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: childrenRect.height

        Column {
            id: outputPanel
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height

            StyledTextLabel {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 8
                height: 40
                font: ui.theme.bodyBoldFont
                text: "MIDI output"
            }

            StyledTextLabel {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 8
                height: 40
                horizontalAlignment: Text.AlignLeft
                font: ui.theme.bodyBoldFont
                text: "Devices:"
            }

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                height: childrenRect.height

                Repeater {
                    id: outputDevicesRepeater
                    model: midiModel.outputDevices()
                    delegate: Item {

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        height: 40

                        StyledTextLabel {
                            id: label
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            width: 240
                            horizontalAlignment: Text.AlignLeft
                            text: modelData.id + " " + modelData.name
                        }

                        FlatButton {
                            id: connBtn
                            anchors.left: label.right
                            anchors.verticalCenter: parent.verticalCenter
                            height: 30
                            text: modelData.action
                            onClicked: midiModel.outputDeviceAction(modelData.id, modelData.action)
                        }

                        StyledTextLabel {
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.left: connBtn.right
                            anchors.right: parent.right
                            anchors.leftMargin: 8
                            horizontalAlignment: Text.AlignLeft
                            text: modelData.error
                        }
                    }
                }
            }
        }

        Column {
            id: inputPanel
            anchors.top: outputPanel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 16
            height: childrenRect.height

            StyledTextLabel {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 8
                height: 40
                font: ui.theme.bodyBoldFont
                text: "MIDI input"
            }

            StyledTextLabel {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 8
                height: 40
                horizontalAlignment: Text.AlignLeft
                font: ui.theme.bodyBoldFont
                text: "Devices:"
            }

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                height: childrenRect.height

                Repeater {
                    id: inputDevicesRepeater
                    model: midiModel.inputDevices()
                    delegate: Item {

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        height: 40

                        StyledTextLabel {
                            id: ilabel
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            width: 240
                            horizontalAlignment: Text.AlignLeft
                            text: modelData.id + " " + modelData.name
                        }

                        FlatButton {
                            id: iconnBtn
                            anchors.left: ilabel.right
                            anchors.verticalCenter: parent.verticalCenter
                            height: 30
                            text: modelData.action
                            onClicked: midiModel.inputDeviceAction(modelData.id, modelData.action)
                        }

                        StyledTextLabel {
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.left: iconnBtn.right
                            anchors.right: parent.right
                            anchors.leftMargin: 8
                            horizontalAlignment: Text.AlignLeft
                            text: modelData.error
                        }
                    }
                }
            }
        }

        Column {
            anchors.top: inputPanel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 16
            height: childrenRect.height

            FlatButton {
                text: "generate MIDI2.0"
                onClicked: midiModel.generateMIDI20();
            }

            Repeater {
                id: inputEventsRepeator
                model: midiModel.inputEvents()
                delegate: StyledTextLabel {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    height: 20
                    text: modelData
                }
            }
        }
    }
}
