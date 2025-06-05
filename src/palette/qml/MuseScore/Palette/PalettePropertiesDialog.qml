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
import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Palette

import "internal"

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

    NavigationPanel {
        id: navPanel
        name: "PalettePropertiesDialog"
        section: root.navigationSection
        enabled: root.enabled && root.visible
        order: 1
        direction: NavigationPanel.Horizontal
    }

    onNavigationActivateRequested: {
        nameField.navigation.requestActive()
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
            id: nameField
            currentText: propertiesModel.name

            onTextChanged: function(newTextValue) {
                propertiesModel.name = newTextValue
            }

            navigation.panel: navPanel
            navigation.order: 1
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

            PalettePropertyItem {
                title: qsTrc("palette", "Width")
                value: propertiesModel.cellWidth
                incrementStep: 1
                minValue: 1
                maxValue: 500

                onValueEdited: function(newValue) {
                    propertiesModel.cellWidth = newValue
                }

                navigation.panel: navPanel
                navigation.order: 2
            }

            PalettePropertyItem {
                title: qsTrc("palette", "Height")
                value: propertiesModel.cellHeight
                incrementStep: 1
                minValue: 1
                maxValue: 500

                onValueEdited: function(newValue) {
                    propertiesModel.cellHeight = newValue
                }

                navigation.panel: navPanel
                navigation.order: 3
            }

            PalettePropertyItem {
                title: qsTrc("palette", "Element offset")
                value: propertiesModel.elementOffset
                measureUnit: qsTrc("global", "sp")
                incrementStep: 0.1
                minValue: -10
                maxValue: 10

                onValueEdited: function (newValue) {
                    propertiesModel.elementOffset = newValue
                }

                navigation.panel: navPanel
                navigation.order: 4
            }

            PalettePropertyItem {
                title: qsTrc("palette", "Scale")
                value: propertiesModel.scaleFactor
                incrementStep: 0.1
                minValue: 0.1
                maxValue: 15

                onValueEdited: function (newValue) {
                    propertiesModel.scaleFactor = newValue
                }

                navigation.panel: navPanel
                navigation.order: 5
            }
        }

        CheckBox {
            width: parent.width
            text: qsTrc("palette", "Show grid")

            checked: propertiesModel.showGrid

            onClicked: {
                propertiesModel.showGrid = !checked
            }

            navigation.panel: navPanel
            navigation.order: 6
        }

        ButtonBox {
            width: parent.width

            buttons: [ ButtonBoxModel.Cancel, ButtonBoxModel.Ok ]

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

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
