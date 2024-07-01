/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import QtQuick.Layouts
import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"
import "internal"

Item {
    id: root

    property alias model: fretDiagramTabPanel.model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "FretDiagramSettings"

    height: content.implicitHeight

    function focusOnFirst() {
        fretDiagramTabPanel.focusOnFirst()
    }

    Column {
        id: content

        width: parent.width

        spacing: 12

        FretDiagramTabPanel {
            id: fretDiagramTabPanel

            width: parent.width

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }

        Column {
            height: childrenRect.height
            width: parent.width
            spacing: 12

            FretCanvas {
                id: fretCanvas

                diagram: root.model ? root.model.fretDiagram : null
                isBarreModeOn: root.model ? root.model.isBarreModeOn : false
                isMultipleDotsModeOn: root.model ? root.model.isMultipleDotsModeOn : false
                currentFretDotType: root.model ? root.model.currentFretDotType : false
                visible: root.model ? root.model.areSettingsAvailable : false
                color: ui.theme.fontPrimaryColor

                width: parent.width
            }

            PropertyToggle {
                visible: root.model ? root.model.areSettingsAvailable : false
                id: showFingerings
                text: qsTrc("inspector", "Show fingerings")
                propertyItem: root.model ? root.model.showFingerings : null
            }

            GridLayout {
                visible: root.model ? root.model.areSettingsAvailable && root.model.showFingerings.value : false
                width: parent.width
                columns: 6
                Repeater {
                    id: rep
                    model: root.model ? root.model.fingerings : 0
                    Column {
                        property int string: rep.count - index - 1
                        Layout.preferredWidth: 40
                        spacing: 8
                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            height: numberLabel.height + 4
                            width: height

                            color: "transparent"
                            radius: 180
                            border.color: ui.theme.fontPrimaryColor
                            border.width: 1

                            StyledTextLabel {
                                id: numberLabel
                                anchors.centerIn: parent
                                text: string + 1
                            }
                        }
                        TextInputField {
                            id: fingerInput
                            textHorizontalAlignment: Qt.AlignHCenter
                            indeterminateText: '-'
                            isIndeterminate: modelData == '0'
                            currentText: modelData == '0' ? '' : modelData
                            onTextChanged: {
                                var newFinger = parseInt(newTextValue)
                                if (isNaN(newFinger)) {
                                    fingerInput.currentText = ''
                                    return;
                                }
                            }
                            onTextEditingFinished: {
                                var newFinger = parseInt(newTextValue)
                                if (root.model) {
                                    root.model.setFingering(string, newFinger)
                                }
                                currentText = modelData
                            }
                        }
                    }
                }
            }


            FlatButton {
                width: parent.width

                visible: root.model ? root.model.areSettingsAvailable : false

                text: qsTrc("global", "Clear")

                navigation.name: "Clear"
                navigation.panel: root.navigationPanel
                navigation.row: 10000

                onClicked: {
                    fretCanvas.clear()
                    root.model.fretNumber.resetToDefault()
                    root.model.resetFingerings()
                }
            }
        }
    }

    StyledTextLabel {
        anchors.fill: parent

        wrapMode: Text.Wrap
        text: qsTrc("inspector", "You have multiple fretboard diagrams selected. Select a single diagram to edit its settings.")
        visible: root.model ? !root.model.areSettingsAvailable &&  fretDiagramTabPanel.isGeneralTabOpen : false
    }
}
