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
import MuseScore.Ui 1.0
import MuseScore.AppShell 1.0

import MuseScore.Pianoroll 1.0

ColumnLayout {

    property int m_graphWidth: 0
    property int m_tuplet: 1
    property int m_subdivision: 0
    property int m_controlWidth: 0
    property double m_centerX: 0
    property double m_wholeNoteWidth: 20

    RowLayout {
        spacing: 0
        Layout.fillWidth: true

        ColumnLayout {
            Layout.preferredWidth: m_controlWidth
            Layout.maximumWidth: m_controlWidth
            Layout.fillHeight: true

            ComboBox {
                id: automationAttribute
                textRole: "text"
                valueRole: "value"

                model: [
                    { text: qsTr("Velocity"), value: PianorollAutomationEditor.VELOCITY },
                    { text: qsTr("Expression"), value: PianorollAutomationEditor.EXPRESSION },
                    { text: qsTr("Pan"), value: PianorollAutomationEditor.PAN }
                ]

                onActivated: {
                    automationEditor.automationType = currentValue
                    //pianoView.tuplet = currentValue
                    if (currentValue === PianorollAutomationEditor.VELOCITY) {
                        automationEditor.visible = true
                        automationCurves.visible = false
                    }
                    else {
                        automationEditor.visible = false
                        automationCurves.visible = true
                    }
                }
            }

        }

        PianorollAutomationEditor {
            id: automationEditor
            Layout.preferredWidth: m_graphWidth
            Layout.fillHeight: true

            tuplet: m_tuplet
            subdivision: m_subdivision

            centerX: m_centerX

            wholeNoteWidth: m_wholeNoteWidth

            Component.onCompleted: {
                load()
            }
        }

        PianorollAutomationCurves {
            id: automationCurves
            Layout.preferredWidth: m_graphWidth
            Layout.fillHeight: true

            tuplet: m_tuplet
            subdivision: m_subdivision

            centerX: m_centerX

            wholeNoteWidth: m_wholeNoteWidth

            Component.onCompleted: {
                load()
            }
        }

//        Label {
//            id: automationCurves
//            visible: false
//            Layout.preferredWidth: m_graphWidth
//            Layout.fillHeight: true
//            Text {
//                text: qsTr("text")
//            }
//        }

        Label {
            Layout.fillHeight: true

            text: " "
        }

    }
}
