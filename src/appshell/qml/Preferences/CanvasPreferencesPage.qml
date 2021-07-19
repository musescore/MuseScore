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
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    Component.onCompleted: {
        preferencesModel.load()
    }

    CanvasPreferencesModel {
        id: preferencesModel
    }

    Column {
        anchors.fill: parent
        spacing: 24

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            StyledTextLabel {
                text: qsTrc("appshell", "Zoom")
                font: ui.theme.bodyBoldFont
            }

            Column {
                spacing: 8

                Row {
                    spacing: 12

                    ComboBoxWithTitle {
                        title: qsTrc("appshell", "Default zoom:")
                        titleWidth: 210

                        control.textRole: "title"
                        control.valueRole: "value"

                        currentIndex: control.indexOfValue(preferencesModel.defaultZoom.type)

                        model: preferencesModel.zoomTypes()

                        onValueEdited: {
                            preferencesModel.setDefaultZoomType(newValue)
                        }
                    }

                    IncrementalPropertyControl {
                        id: defaultZoomControl
                        width: 64
                        iconMode: iconModeEnum.hidden
                        maxValue: 1600
                        minValue: 10
                        step: 10
                        validator: IntInputValidator {
                            top: defaultZoomControl.maxValue
                            bottom: defaultZoomControl.minValue
                        }

                        measureUnitsSymbol: "%"

                        currentValue: preferencesModel.defaultZoom.level
                        enabled: preferencesModel.defaultZoom.isPercentage

                        onValueEdited: {
                            preferencesModel.setDefaultZoomLevel(newValue)
                        }
                    }
                }

                IncrementalPropertyControlWithTitle {
                    title: qsTrc("appshell", "Mouse zoom precision:")

                    titleWidth: 208
                    control.width: 60

                    minValue: 1
                    maxValue: 16
                    currentValue: preferencesModel.mouseZoomPrecision

                    onValueEdited: {
                        preferencesModel.mouseZoomPrecision = newValue
                    }
                }
            }
        }

        SeparatorLine { }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            StyledTextLabel {
                text: qsTrc("appshell", "Scroll Pages")
                font: ui.theme.bodyBoldFont
            }

            Column {
                spacing: 16

                RadioButtonGroup {
                    id: radioButtonList

                    width: 100
                    height: implicitHeight

                    spacing: 12
                    orientation: ListView.Vertical

                    model: [
                        { title: qsTrc("appshell", "Horizontal"), value: Qt.Horizontal },
                        { title: qsTrc("appshell", "Vertical"), value: Qt.Vertical }
                    ]

                    delegate: RoundedRadioButton {
                        width: parent.width
                        leftPadding: 0
                        spacing: 6

                        ButtonGroup.group: radioButtonList.radioButtonGroup

                        checked: preferencesModel.scrollPagesOrientation === modelData["value"]

                        StyledTextLabel {
                            text: modelData["title"]
                            horizontalAlignment: Text.AlignLeft
                        }

                        onToggled: {
                            preferencesModel.scrollPagesOrientation = modelData["value"]
                        }
                    }
                }

                CheckBox {
                    text: qsTrc("appshell", "Limit scroll area to page borders")
                    checked: preferencesModel.limitScrollArea

                    onClicked: {
                        preferencesModel.limitScrollArea = !preferencesModel.limitScrollArea
                    }
                }
            }
        }
    }
}
