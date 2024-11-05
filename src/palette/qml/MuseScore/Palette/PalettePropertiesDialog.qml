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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Palette 1.0

StyledDialogView {
    id: root

    title: qsTrc("palette", "Palette properties")

    contentWidth: 280
    contentHeight: contentColumn.implicitHeight
    margins: 12

    property var properties

    PalettePropertiesModel {
        id: propertiesModel
    }

    Component.onCompleted: {
        propertiesModel.load(properties)
    }

    Column {
        id: contentColumn
        anchors.fill: parent
        spacing: 12

        StyledTextLabel {
            text: qsTrc("palette", "Name")
            font: ui.theme.bodyBoldFont
        }

        TextInputField {
            currentText: propertiesModel.name

            onTextChanged: function(newTextValue) {
                propertiesModel.name = newTextValue
            }
        }

        SeparatorLine { anchors.margins: -parent.margins }

        StyledTextLabel {
            text: qsTrc("palette", "Cell size")
            font: ui.theme.bodyBoldFont
        }

        Grid {
            id: grid
            width: parent.width

            columns: 2
            spacing: 12

            ListModel {
                id: gridModel

                Component.onCompleted: {
                    // NOTE: We must use append() to populate the ListModel because ListItems directly inside
                    // the ListModel do not support property bindings due to Qt limitations.
                    // See https://stackoverflow.com/questions/7659442/listelement-fields-as-properties
                    gridModel.append({
                        title: qsTrc("palette", "Width"),
                        value: propertiesModel.cellWidth,
                        incrementStep: 1,
                        minValue: 1,
                        maxValue: 500
                    })
                    gridModel.append({
                        title: qsTrc("palette", "Height"),
                        value: propertiesModel.cellHeight,
                        incrementStep: 1,
                        minValue: 1,
                        maxValue: 500
                    })
                    gridModel.append({
                        title: qsTrc("palette", "Element offset"),
                        value: propertiesModel.elementOffset,
                        measureUnit: qsTrc("global", "sp"),
                        incrementStep: 0.1,
                        minValue: -10,
                        maxValue: 10
                    })
                    gridModel.append({
                        title: qsTrc("palette", "Scale"),
                        value: propertiesModel.scaleFactor,
                        incrementStep: 0.1,
                        minValue: 0.1,
                        maxValue: 15
                    })

                    propertiesModel.onPropertiesChanged.connect(function () {
                        if (gridModel.count > 0) {
                            gridModel.setProperty(0, "value", propertiesModel.cellWidth)
                            gridModel.setProperty(1, "value", propertiesModel.cellHeight)
                            gridModel.setProperty(2, "value", propertiesModel.elementOffset)
                            gridModel.setProperty(3, "value", propertiesModel.scaleFactor)
                        }
                    })
                }
            }

            Repeater {
                id: repeater

                // This model cannot be a simple array as the Repeater cannot see changes to individual properties
                // of array elements and update only those properties. Instead, it destroys and recreates all of
                // its items which leads to weird issues (and degraded performance of course).
                // See https://github.com/musescore/MuseScore/issues/11530 and https://stackoverflow.com/a/76228394
                model: gridModel

                function setValue(index, value) {
                    if (index === 0) {
                        propertiesModel.cellWidth = value
                    } else if (index === 1) {
                        propertiesModel.cellHeight = value
                    } else if (index === 2) {
                        propertiesModel.elementOffset = value
                    } else if (index === 3) {
                        propertiesModel.scaleFactor = value
                    }
                }

                Column {
                    width: (grid.width - grid.spacing * (grid.columns - 1)) / grid.columns

                    spacing: 8

                    StyledTextLabel {
                        text: model.title
                    }

                    IncrementalPropertyControl {
                        currentValue: model.value
                        measureUnitsSymbol: Boolean(model.measureUnit) ? model.measureUnit : ""
                        step: model.incrementStep
                        minValue: model.minValue
                        maxValue: model.maxValue

                        onValueEdited: function(newValue) {
                            repeater.setValue(model.index, newValue)
                        }
                    }
                }
            }
        }

        CheckBox {
            width: parent.width
            text: qsTrc("palette", "Show grid")

            checked: propertiesModel.showGrid

            onClicked: {
                propertiesModel.showGrid = !checked
            }
        }

        ButtonBox {
            width: parent.width

            buttons: [ ButtonBoxModel.Cancel, ButtonBoxModel.Ok ]

            onStandardButtonClicked: function(buttonId) {
                if (buttonId === ButtonBoxModel.Cancel) {
                    propertiesModel.reject()
                    root.hide()
                } else if (buttonId === ButtonBoxModel.Ok) {
                    root.hide()
                }
            }
        }
    }
}
