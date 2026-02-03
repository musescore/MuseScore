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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project

ExportSettingsPage {
    id: root
    readonly property string exportBeamsTooltip:
        qsTrc("project/export", "Disable to let the importing app choose beam groupings.")
    readonly property string exportRestPositionsTooltip:
        qsTrc("project/export", "Enable to export the staff line of every rest. Disable to let the importing app decide.")

    ExportOptionItem {
        id: indentLabel
        text: qsTrc("project/export", "JSON indent:")

        Column {
            width: parent.width
            spacing: 4

            id: indentColumn

            property var indentOptions: []

            Component.onCompleted: {
                var list = []
                list.push({ text: qsTrc("project/export", "No line breaks"), value: -1 })
                for (var i = 0; i <= 8; ++i) {
                    list.push({ text: qsTrc("project/export", "%1 spaces").arg(i), value: i })
                }
                indentOptions = list
            }

            StyledDropdown {
                Layout.fillWidth: true

                navigation.name: "MnxIndentDropdown"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationOrder + 1
                navigation.accessible.name: indentLabel.text + " " + currentText

                model: indentColumn.indentOptions
                textRole: "text"
                valueRole: "value"

                function findCurrentIndex() {
                    if (root.model) {
                        for (var i = 0; i < indentColumn.indentOptions.length; ++i) {
                            if (indentColumn.indentOptions[i].value === root.model.mnxIndentSpaces) {
                                return i
                            }
                        }
                    }
                    return 0
                }

                currentIndex: findCurrentIndex()

                onActivated: function(index, value) {
                    root.model.mnxIndentSpaces = value
                }
            }
        }
    }

    CheckBox {
        id: exportBeamsCheckbox
        width: parent.width
        text: qsTrc("project/export", "Export beams")
        navigation.accessible.description: root.exportBeamsTooltip

        navigation.name: "MnxExportBeamsCheckbox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder + 2

        checked: root.model.mnxExportBeams
        onHoveredChanged: {
            if (hovered) {
                ui.tooltip.show(exportBeamsCheckbox, root.exportBeamsTooltip)
            } else {
                ui.tooltip.hide(exportBeamsCheckbox)
            }
        }
        onPressedChanged: {
            if (pressed) {
                ui.tooltip.hide(exportBeamsCheckbox, true)
            }
        }
        onClicked: root.model.mnxExportBeams = !checked
    }

    CheckBox {
        id: exportRestPositionsCheckbox
        width: parent.width
        text: qsTrc("project/export", "Export rest positions")
        navigation.accessible.description: root.exportRestPositionsTooltip

        navigation.name: "MnxExportRestPositionsCheckbox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder + 3

        checked: root.model.mnxExportRestPositions
        onHoveredChanged: {
            if (hovered) {
                ui.tooltip.show(exportRestPositionsCheckbox, root.exportRestPositionsTooltip)
            } else {
                ui.tooltip.hide(exportRestPositionsCheckbox)
            }
        }
        onPressedChanged: {
            if (pressed) {
                ui.tooltip.hide(exportRestPositionsCheckbox, true)
            }
        }
        onClicked: root.model.mnxExportRestPositions = !checked
    }
}
