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

                // TODO: keyboard navigation
            }

            PropertyToggle {
                id: showFingerings

                visible: root.model ? root.model.areSettingsAvailable : false

                text: qsTrc("inspector", "Show fingerings")
                propertyItem: root.model ? root.model.showFingerings : null

                navigation.name: "Show fingerings toggle"
                navigation.panel: root.navigationPanel
                navigation.row: fretDiagramTabPanel.navigationRowEnd + 1
            }

            GridLayout {
                visible: root.model ? root.model.areSettingsAvailable && root.model.showFingerings.value : false
                width: parent.width
                columns: 6
                rowSpacing: 12

                Repeater {
                    id: repeater

                    readonly property int navigationRowStart: showFingerings.navigation.row + 1
                    readonly property int navigationRowEnd: navigationRowStart + repeater.count - 1

                    //! NOTE: If we put `root.model.fingerings` here, the repeater would destroy all generated items
                    //! whenever one of the fingerings is changed. This results in focus being lost when clicking
                    //! on a second TextInputField after editing one, instead of focussing that second TextInputField.
                    //! By giving the repeater only an integer value, that happens only when the number of items changes,
                    //! which is less problematic.
                    model: root.model ? root.model.fingerings.length : 0

                    Column {
                        id: repeaterItem

                        property int string: repeater.count - index - 1
                        property string finger: root.model ? root.model.fingerings[string] : 0

                        Layout.preferredWidth: 40
                        spacing: 8

                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            height: numberLabel.height + 4
                            width: height

                            color: "transparent"
                            radius: height / 2
                            border.color: ui.theme.fontPrimaryColor
                            border.width: 1

                            StyledTextLabel {
                                id: numberLabel
                                anchors.centerIn: parent
                                text: repeaterItem.string + 1
                            }
                        }

                        TextInputField {
                            id: fingerInput

                            textHorizontalAlignment: Qt.AlignHCenter
                            indeterminateText: '-'
                            isIndeterminate: {
                                const fingerInt = parseInt(repeaterItem.finger)
                                return isNaN(fingerInt) || fingerInt < 1 || fingerInt > 5
                            }

                            currentText: isIndeterminate ? '' : repeaterItem.finger

                            validator: IntInputValidator {
                                top: 5
                                bottom: 0
                            }

                            navigation.name: `Finger ${repeaterItem.string + 1} text input`
                            navigation.panel: root.navigationPanel
                            navigation.row: repeater.navigationRowStart + index
                            navigation.accessible.name: qsTrc("inspector", "Finger for string %1").arg(repeaterItem.string + 1)

                            onTextEditingFinished: function (newTextValue) {
                                var newFinger = parseInt(newTextValue)
                                if (root.model) {
                                    root.model.setFingering(repeaterItem.string, newFinger)
                                }
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
                navigation.row: repeater.navigationRowEnd + 1

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
