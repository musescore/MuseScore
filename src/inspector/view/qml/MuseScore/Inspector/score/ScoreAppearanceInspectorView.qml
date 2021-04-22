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
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn

        width: parent.width

        spacing: 24

        GridLayout {
            id: grid

            width: parent.width

            columns: 2

            rowSpacing: 16
            columnSpacing: 4

            Column {
                spacing: 8

                Layout.fillWidth: true
                Layout.maximumWidth: parent.width/2

                StyledTextLabel {
                    text: qsTrc("inspector", "Page size")
                }

                StyledComboBox {
                    width: parent.width

                    navigation.panel: root.navigationPanel
                    navigation.name: "Page size"
                    navigation.row: root.navigationRow(1)

                    textRoleName: "nameRole"
                    valueRoleName: "idRole"

                    model: root.model ? root.model.pageTypeListModel : null

                    currentIndex: root.model && root.model.pageTypeListModel ? indexOfValue(root.model.pageTypeListModel.currentPageSizeId) : -1

                    onValueChanged: {
                        root.model.pageTypeListModel.currentPageSizeId = value
                    }
                }
            }

            Column {
                spacing: 8

                Layout.fillWidth: true
                Layout.maximumWidth: parent.width/2

                StyledTextLabel {
                    text: qsTrc("inspector", "Orientation")
                }

                RadioButtonGroup {
                    id: orientationType

                    height: 30
                    width: parent.width

                    model: [
                        { iconRole: IconCode.ORIENTATION_PORTRAIT, typeRole: ScoreAppearanceTypes.ORIENTATION_PORTRAIT },
                        { iconRole: IconCode.ORIENTATION_LANDSCAPE, typeRole: ScoreAppearanceTypes.ORIENTATION_LANDSCAPE }
                    ]

                    delegate: FlatRadioButton {

                        ButtonGroup.group: orientationType.radioButtonGroup

                        navigation.panel: root.navigationPanel
                        navigation.name: "Orientation"+model.index
                        navigation.row: root.navigationRow(model.index + 2)

                        checked: root.model ? root.model.orientationType === modelData["typeRole"] : false
                        onToggled: {
                            root.model.orientationType = modelData["typeRole"]
                        }

                        StyledIconLabel {
                            iconCode: modelData["iconRole"]
                        }
                    }
                }
            }

            Column {
                spacing: 8

                Layout.fillWidth: true
                Layout.maximumWidth: parent.width/2

                StyledTextLabel {
                    text: qsTrc("inspector", "Staff spacing")
                }

                IncrementalPropertyControl {
                    iconMode: iconModeEnum.hidden

                    navigation.panel: root.navigationPanel
                    navigation.name: "Staff spacing"
                    navigation.row: root.navigationRow(4)

                    currentValue: root.model ? root.model.staffSpacing : 0
                    measureUnitsSymbol: qsTrc("inspector", "mm")

                    step: 0.1
                    decimals: 3
                    minValue: 0.1
                    maxValue: 100

                    onValueEdited: { root.model.staffSpacing = newValue }
                }
            }

            Column {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width/2

                spacing: 8

                StyledTextLabel {
                    text: qsTrc("inspector", "Staff distance")
                }

                IncrementalPropertyControl {
                    iconMode: iconModeEnum.hidden

                    navigation.panel: root.navigationPanel
                    navigation.name: "Staff distance"
                    navigation.row: root.navigationRow(5)

                    currentValue: root.model ? root.model.staffDistance : 0

                    step: 0.1
                    minValue: 0.1
                    maxValue: 10

                    onValueEdited: { root.model.staffDistance = newValue }
                }
            }
        }

        Column {
            width: parent.width

            spacing: 8

            FlatButton {
                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.name: "More page settings"
                navigation.row: root.navigationRow(6)

                text: qsTrc("inspector", "More page settings")

                onClicked: {
                    if (root.model) {
                        root.model.showPageSettings()
                    }
                }
            }

            FlatButton {
                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.name: "Style settings"
                navigation.row: root.navigationRow(7)

                text: qsTrc("inspector", "Style settings")

                onClicked: {
                    if (root.model) {
                        root.model.showStyleSettings()
                    }
                }
            }
        }
    }
}
