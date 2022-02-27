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

import MuseScore.Project 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Column {
    id: root

    required property ExportDialogModel exportModel
    property alias navigation: navPanel

    spacing: 12

    NavigationPanel {
        id: navPanel
        name: "ExportOptions"
        enabled: root.visible && root.enabled
        direction: NavigationPanel.Vertical
    }

    ExportOptionItem {
        id: typeLabel
        width: parent.width
        text: qsTrc("project", "Format:")

        Dropdown {
            id: typeComboBox
            Layout.fillWidth: true

            navigation.name: "ExportTypeDropdown"
            navigation.panel: navPanel
            navigation.row: 1
            navigation.accessible.name: typeLabel.text + " " + currentText

            model: exportModel.exportTypeList()
            popupItemsCount: typeComboBox.count

            textRole: "name"
            valueRole: "id"

            currentIndex: {
                // First, check if it's a subtype
                const index = model.findIndex(function(type) {
                    return type.subtypes.some(function(subtype) {
                        return subtype.id === exportModel.selectedExportType.id
                    })
                })

                if (index !== -1) {
                    return index
                }

                // Otherwise, it must be a toplevel type
                return typeComboBox.indexOfValue(exportModel.selectedExportType.id)
            }

            onCurrentValueChanged: {
                exportModel.selectExportTypeById(typeComboBox.currentValue)
            }
        }
    }

    ExportOptionItem {
        id: subtypeLabel
        width: parent.width
        visible: subtypeComboBox.count > 0
        text: qsTrc("project", "File type:")

        Dropdown {
            id: subtypeComboBox
            Layout.fillWidth: true

            navigation.name: "ExportSubtypeDropdown"
            navigation.panel: navPanel
            navigation.row: 2
            navigation.accessible.name: subtypeLabel.text + " " + currentText

            model: {
                if (typeComboBox.currentIndex > -1) {
                    return typeComboBox.model[typeComboBox.currentIndex].subtypes
                }

                return []
            }

            textRole: "name"
            valueRole: "id"

            currentIndex: subtypeComboBox.indexOfValue(exportModel.selectedExportType.id)
            onCurrentValueChanged: {
                exportModel.selectExportTypeById(subtypeComboBox.currentValue)
            }
        }
    }

    Loader {
        id: pageLoader
        width: parent.width

        Connections {
            target: root.exportModel

            function onSelectedExportTypeChanged() {
                if (!root.exportModel.selectedExportType.settingsPagePath) {
                    pageLoader.setSource("")
                }

                var properties = {
                    model: Qt.binding(() => root.exportModel),
                    navigationPanel: navPanel,
                    navigationOrder: 3
                }

                pageLoader.setSource(root.exportModel.selectedExportType.settingsPagePath, properties)
            }
        }
    }

    RadioButtonGroup {
        width: parent.width
        visible: count > 1
        spacing: 12
        orientation: Qt.Vertical

        model: exportModel.availableUnitTypes

        delegate: RoundedRadioButton {
            text: modelData["text"]

            navigation.name: "ExportType_" + text
            navigation.panel: navPanel
            navigation.row: 100000 + model.index

            checked: exportModel.selectedUnitType === modelData["value"]
            onToggled: {
                exportModel.selectedUnitType = modelData["value"]
            }
        }
    }
}
