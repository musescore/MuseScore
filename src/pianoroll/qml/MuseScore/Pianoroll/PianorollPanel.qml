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
                }

                RowLayout {

                    ToolButton {
                        text: qsTr("Automation")
                        checkable: true
                    }

                }


                RowLayout {
//                    anchors.fill: parent

                    Label {
                        text: qsTr("Grid:")
                    }

//                    ComboBox {
//                        id: gridCombo

//                        model: ListModel {
//                            id: gridModel
//                            ListElement {
//                                text: "1/1"
//                            }
//                            ListElement {
//                                text: "1/2"
//                            }
//                            ListElement {
//                                text: "1/4"
//                            }
//                            ListElement {
//                                text: "1/8"
//                            }
//                        }
//                    }

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
                            { text: qsTr("Triplet"), value: 3 }
                        ]

                        onActivated: {
                            pianoView.tuplet = currentValue
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


        GridLayout {
            columns: 3
            //anchors.fill: parent
            Layout.fillHeight: true
            rowSpacing: 0
            columnSpacing: 0


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

//            ScrollBar {
//                id: hbar
//                hoverEnabled: true
//                active: hovered || pressed
//                orientation: Qt.Horizontal
//                size: 20 //
//                //Layout.preferredWidth:
//                Layout.fillWidth: true
//                Layout.minimumHeight: 12
//            }

//            Text {
//                Layout.minimumWidth: 12
//                Layout.minimumHeight: 12
//                text: "words"; font.bold: true;
//            }
//            Text {
//                Layout.minimumWidth: 12
//                Layout.minimumHeight: 12
//                text: "in"; font.bold: true;
//            }
        }

//        ScrollView {
//            Layout.fillWidth: true
//            Layout.fillHeight: true
//            clip: true
//            ScrollBar.horizontal.interactive: true
//            ScrollBar.vertical.interactive: true
//            ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
//            ScrollBar.vertical.policy: ScrollBar.AlwaysOn

//        }
    }
}
angle {
    color: ui.theme.backgroundPrimaryColor

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
                        text: qsTr("Edit")
                        checkable: true

                        onClicked: {
                            pianoView.tool = PianorollView.EDIT
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
                }

                RowLayout {

                    ToolButton {
                        text: qsTr("Automation")
                        checkable: true
                    }

//                    FlatToggleButton {
//                        id: automationButton

//                        icon: checked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN
//                        height: 20
//                        width: 20

//                        checked: true

////                        onToggled: {
////                            root.model.isAspectRatioLocked.value = !root.model.isAspectRatioLocked.value
////                        }
//                    }

                }


                RowLayout {
//                    anchors.fill: parent

                    Label {
                        text: qsTr("Grid:")
                    }

                    ComboBox {
                        id: gridCombo

                        model: ListModel {
                            id: gridModel
                            ListElement {
                                text: "1/1"
                            }
                            ListElement {
                                text: "1/2"
                            }
                            ListElement {
                                text: "1/4"
                            }
                            ListElement {
                                text: "1/8"
                            }
                        }
                    }

                    Label {
                        text: qsTr("Tuplet:")
                    }

                    ComboBox {
                        id: tupletCombo
                        model: ListModel {
                            ListElement {
                                text: qsTr("None")
                            }
                            ListElement {
                                text: qsTr("Duplet")
                            }
                            ListElement {
                                text: qsTr("Triplet")
                            }
                        }
                        onObjectNameChanged: {

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
                        to: 40
                        value: 14
                    }

                    Label {
                        text: "H:"
                    }

                    Slider {
                        id: horizZoom
                        from: 1
                        to: 100
                        value: 20
                    }
                }

            }
        }



        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ScrollBar.horizontal.interactive: true
            ScrollBar.vertical.interactive: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn

            PianorollView {
                id: pianoView
                anchors.fill: parent

                implicitWidth: 1500
                implicitHeight: 500

//                zoomX: horizZoom.value
//                zoomX: Math.log(horizZoom.value) / Math.log(2)
                wholeNoteWidth: Math.pow(horizZoom.value, 1.1)
                noteHeight: vertZoom.value
                tool: PianorollView.SELECT


                StyledTextLabel {
                    anchors.centerIn: parent
                    text: pianoRollPanel.title + " Walla"
                }

                Component.onCompleted: {
                    load()
                }

            }
        }
    }
}
