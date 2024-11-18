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

    title: qsTrc("palette", "Palette cell properties")

    contentWidth: 280
    contentHeight: contentColumn.implicitHeight
    margins: 12

    property var properties

    PaletteCellPropertiesModel {
        id: propertiesModel
    }

    Component.onCompleted: {
        propertiesModel.load(root.properties)
    }

    NavigationPanel {
        id: navPanel
        name: "PaletteCellPropertiesDialog"
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
            text: qsTrc("palette", "Content offset")
            font: ui.theme.bodyBoldFont
        }

        Grid {
            id: grid
            width: parent.width

            columns: 2
            spacing: 12

            PalettePropertyItem {
                title: qsTrc("palette", "X")
                value: propertiesModel.xOffset
                incrementStep: 1
                minValue: -10
                maxValue: 10
                //: Abbreviation of "spatium"
                measureUnit: qsTrc("global", "sp")

                onValueEdited: function (newValue) {
                    propertiesModel.xOffset = newValue
                }

                navigation.panel: navPanel
                navigation.order: 2
            }

            PalettePropertyItem {
                title: qsTrc("palette", "Y")
                value: propertiesModel.yOffset
                incrementStep: 1
                minValue: -10
                maxValue: 10
                measureUnit: qsTrc("global", "sp")

                onValueEdited: function (newValue) {
                    propertiesModel.yOffset = newValue
                }

                navigation.panel: navPanel
                navigation.order: 3
            }

            PalettePropertyItem {
                title: qsTrc("palette", "Content scale")
                value: propertiesModel.scaleFactor
                incrementStep: 0.1
                minValue: 0.1
                maxValue: 10

                onValueEdited: function (newValue) {
                    propertiesModel.scaleFactor = newValue
                }

                navigation.panel: navPanel
                navigation.order: 4
            }
        }

        CheckBox {
            width: parent.width
            text: qsTrc("palette", "Draw staff")

            checked: propertiesModel.drawStaff

            onClicked: {
                propertiesModel.drawStaff = !checked
            }

            navigation.panel: navPanel
            navigation.order: 5
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
