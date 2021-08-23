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

                    FlatToggleButton {
                        icon: IconCode.COPY

//                        onToggled: {
//                            root.model.isAspectRatioLocked.value = !root.model.isAspectRatioLocked.value
//                        }
                    }

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
                        //onClicked
                    }
                    ToolButton {
                        text: qsTr("Edit")
                        checkable: true

                    }
                    ToolButton {
                        text: qsTr("Cut")
                        checkable: true
                    }
                    ToolButton {
                        text: qsTr("Erase")
                        checkable: true
                    }
                }

                RowLayout {
//                    anchors.fill: parent

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
                        from: 1
                        to: 100
                        value: 20
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

            contentWidth: 1500
            contentHeight: 500

            PianorollView {

                implicitWidth: 1500
                implicitHeight: 500

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
