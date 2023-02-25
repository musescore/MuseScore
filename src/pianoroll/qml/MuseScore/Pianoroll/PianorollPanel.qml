﻿/*
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

Rectangle {
    color: ui.theme.backgroundPrimaryColor
//    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent


        ToolBar {
            RowLayout
            {
                anchors.fill: parent

                ButtonGroup {
                    buttons: tools.children
                }

                RowLayout {
                    id: tools

                    ToolButton {
                        text: qsTr("Select")
                        icon: IconCode.COPY
                        ToolTip.text: qsTr("Select")
                        hoverEnabled: true
                        checkable: true
                        checked: true

                        ToolTip.delay: 1000
                        ToolTip.timeout: 5000
                        ToolTip.visible: hovered

                        onClicked: {
                            pianoView.tool = PianorollView.SELECT
                        }
                    }

                    ToolButton {
                        text: qsTr("Add")
                        checkable: true

                        onClicked: {
                            pianoView.tool = PianorollView.ADD
                        }
                    }

                    ToolButton {
                        text: qsTr("Cut")
                        checkable: true

                        onClicked: {
                            pianoView.tool = PianorollView.CUT
                        }
                    }

                    ToolButton {
                        text: qsTr("Erase")
                        checkable: true

                        onClicked: {
                            pianoView.tool = PianorollView.ERASE
                        }
                    }

                    ToolButton {
                        text: qsTr("Event Adjust")
                        checkable: true

                        onClicked: {
                            pianoView.tool = PianorollView.EVENT_ADJUST
                        }
                    }
                }

                RowLayout {
                    id: automationButtons

                    ToolButton {
                        text: qsTr("Automation")
                        checkable: true

                        onClicked: {
                            automationArea.visible = checked
                        }
                    }
                }

                RowLayout {
                    Label {
                        text: qsTr("Grid:")
                    }

                    ComboBox {
                        id: gridCombo
                        textRole: "text"
                        valueRole: "value"

                        model: [
                            { text: "1/1", value: 0 },
                            { text: "1/2", value: 1 },
                            { text: "1/4", value: 2 },
                            { text: "1/8", value: 3 }
                        ]

                        onActivated: {
                            pianoView.subdivision = currentValue
                        }
                    }

                    Label {
                        text: qsTr("Tuplet:")
                    }

                    ComboBox {
                        id: tupletCombo
                        textRole: "text"
                        valueRole: "value"

                        model: [
                            { text: qsTr("None"), value: 1 },
                            { text: qsTr("Duplet"), value: 2 },
                            { text: qsTr("Triplet"), value: 3 },
                            { text: qsTr("Quadruplet"), value: 4 },
                            { text: qsTr("Quintuplet"), value: 5 }
                        ]

                        onActivated: {
                            pianoView.tuplet = currentValue
                        }
                    }

                    Label {
                        text: qsTr("Stripes:")
                    }

                    ComboBox {
                        id: stripePatternCombo
                        textRole: "text"
                        valueRole: "value"

                        model: [
                            { text: qsTr("C major / A minor"), value: PianorollView.C_MAJ },
                            { text: qsTr("D♭ major / B♭ minor"), value: PianorollView.DF_MAJ },
                            { text: qsTr("D major / B minor"), value: PianorollView.D_MAJ },
                            { text: qsTr("E♭ major / C minor"), value: PianorollView.DF_MAJ },
                            { text: qsTr("E major / C♯ minor"), value: PianorollView.E_MAJ },
                            { text: qsTr("F major / D minor"), value: PianorollView.F_MAJ },
                            { text: qsTr("G♭ major / E♭ minor"), value: PianorollView.GF_MAJ },
                            { text: qsTr("G major / E minor"), value: PianorollView.G_MAJ },
                            { text: qsTr("A♭ major / F minor"), value: PianorollView.AF_MAJ },
                            { text: qsTr("A major / F♯ minor"), value: PianorollView.A_MAJ },
                            { text: qsTr("B♭ major / G minor"), value: PianorollView.BF_MAJ },
                            { text: qsTr("B major / G♯ minor"), value: PianorollView.B_MAJ },
                            { text: qsTr("C Diminished"), value: PianorollView.C_DIM },
                            { text: qsTr("D♭ Diminished"), value: PianorollView.DF_DIM },
                            { text: qsTr("D Diminished"), value: PianorollView.D_DIM },
                            { text: qsTr("C Half/Whole"), value: PianorollView.C_HW },
                            { text: qsTr("D♭ Half/Whole"), value: PianorollView.DF_HW },
                            { text: qsTr("D Half/Whole"), value: PianorollView.D_HW },
                            { text: qsTr("C Whole tone"), value: PianorollView.C_W },
                            { text: qsTr("D♭ Whole tone"), value: PianorollView.DF_W },
                            { text: qsTr("C Augmented"), value: PianorollView.C_AUG },
                            { text: qsTr("D♭ Augmented"), value: PianorollView.DF_AUG },
                            { text: qsTr("D Augmented"), value: PianorollView.D_AUG },
                            { text: qsTr("E♭ Augmented"), value: PianorollView.EF_AUG },
                            { text: qsTr("C major / A minor pentatonic"), value: PianorollView.C_MAJ_P },
                            { text: qsTr("D♭ major / B♭ minor pentatonic"), value: PianorollView.DF_MAJ_P },
                            { text: qsTr("D major / B minor pentatonic"), value: PianorollView.D_MAJ_P },
                            { text: qsTr("E♭ major / C minor pentatonic"), value: PianorollView.EF_MAJ_P },
                            { text: qsTr("E major / C♯ minor pentatonic"), value: PianorollView.E_MAJ_P },
                            { text: qsTr("F major / D minor pentatonic"), value: PianorollView.F_MAJ_P },
                            { text: qsTr("G♭ major / E♭ minor pentatonic"), value: PianorollView.GF_MAJ_P },
                            { text: qsTr("G major / E minor pentatonic"), value: PianorollView.G_MAJ_P },
                            { text: qsTr("A♭ major / F minor pentatonic"), value: PianorollView.AF_MAJ_P },
                            { text: qsTr("A major / F♯ minor pentatonic"), value: PianorollView.A_MAJ_P },
                            { text: qsTr("B♭ major / G minor"), value: PianorollView.BF_MAJ_P },
                            { text: qsTr("B major / G♯ minor pentatonic"), value: PianorollView.B_MAJ_P }
                        ]

                        onActivated: {
                            pianoView.stripePattern = currentValue
                        }
                    }
                }

                RowLayout {
                    anchors.right: parent.right

                    Label {
                        text: "V:"
                    }

                    Slider {
                        id: vertZoom
                        from: 4
                        to: 30
                        value: 14
                    }

                    Label {
                        text: "H:"
                    }

                    Slider {
                        id: horizZoom
                        from: 20
                        to: 800
                        value: 60
                    }
                }

            }
        }

        SplitView {
            //anchors.fill: parent
            Layout.fillHeight: true
            Layout.fillWidth: true
            orientation: Qt.Vertical

            GridLayout {
                columns: 3
                rowSpacing: 0
                columnSpacing: 0
                SplitView.minimumHeight: 100
                SplitView.fillHeight: true

                //-------
                //Row
                Text {
                    Layout.minimumWidth: 12
                    Layout.minimumHeight: 12
                    text: "";
                }

                PianorollRuler {
                    Layout.fillWidth: true
                    height: 30

                    centerX: scrollViewX.center
                    wholeNoteWidth: horizZoom.value

                    Component.onCompleted: {
                        load()
                    }
                }

                Text {
                    Layout.minimumWidth: 12
                    Layout.minimumHeight: 12
                    text: "";
                }

                //-------
                //Row
                PianorollKeyboard {
                    id: keyboardComponent
                    width: 120
                    Layout.fillHeight: true

                    centerY: scrollViewY.center
                    noteHeight: vertZoom.value

                    Component.onCompleted: {
                        load()
                    }
                }

                PianorollView {
                    id: pianoView
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    implicitWidth: 1500
                    implicitHeight: 500

                    centerX: scrollViewX.center
                    centerY: scrollViewY.center

                    wholeNoteWidth: horizZoom.value
                    noteHeight: vertZoom.value
                    tool: PianorollView.SELECT

                    Component.onCompleted: {
                        load()
                    }
                }

                PianorollScrollbar{
                    id: scrollViewY
                    diretion: PianorollScrollbar.VERTICAL
                    displayObjectSpan: pianoView.displayObjectHeight
                    viewportSpan: pianoView.height
                    Layout.minimumWidth: 12
                    Layout.fillHeight: true
                }

                //-------
                //Row
                Text {
                    Layout.minimumWidth: 12
                    Layout.minimumHeight: 12
                    text: "";
                }

                PianorollScrollbar{
                    id: scrollViewX
                    diretion: PianorollScrollbar.HORIZONTAL
                    displayObjectSpan: pianoView.displayObjectWidth
                    viewportSpan: pianoView.width
                    Layout.fillWidth: true
                    Layout.minimumHeight: 12
                }

                Text {
                    Layout.minimumWidth: 12
                    Layout.minimumHeight: 12
                    text: "";
                }
            }

            PianorollAutomationRow {
                id: automationArea
                SplitView.preferredHeight: 100
                visible: false

                m_graphWidth: pianoView.width
                m_tuplet: pianoView.tuplet
                m_subdivision: pianoView.subdivision
                m_controlWidth: keyboardComponent.width
                m_centerX: scrollViewX.center
                m_wholeNoteWidth: horizZoom.value

                onRowAddClicked: {
                }

                onRowRemoveClicked: {
                }
            }
        }
    }
}
