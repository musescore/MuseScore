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
import QtQuick 2
import QtQuick.Controls 2
import QtQuick.Layouts 1

import MuseScore.UserScores 1
import MuseScore.UiComponents 1

ColumnLayout {
    id: root

    required property ExportDialogModel exportModel

    spacing: 12

    QtObject {
        id: privateProperties

        readonly property int firstColumnWidth: 72
    }

    ExportOptionItem {
        firstColumnWidth: privateProperties.firstColumnWidth
        text: qsTrc("userscores", "Format:")

        StyledComboBox {
            id: typeComboBox
            Layout.fillWidth: true

            model: exportModel.exportTypeList()
            maxVisibleItemCount: count

            textRoleName: "name"
            valueRoleName: "id"

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
                return indexOfValue(exportModel.selectedExportType.id)
            }

            onValueChanged: {
                exportModel.selectExportTypeById(value)
            }
        }
    }

    ExportOptionItem {
        visible: subtypeComboBox.count > 0
        text: qsTrc("userscores", "File type:")
        firstColumnWidth: privateProperties.firstColumnWidth

        StyledComboBox {
            id: subtypeComboBox
            Layout.fillWidth: true

            model: {
                if (typeComboBox.currentIndex > -1) {
                    return typeComboBox.model[typeComboBox.currentIndex].subtypes
                }

                return []
            }

            textRoleName: "name"
            valueRoleName: "id"

            currentIndex: indexOfValue(exportModel.selectedExportType.id)
            onValueChanged: {
                exportModel.selectExportTypeById(value)
            }
        }
    }

    Loader {
        source: exportModel.selectedExportType.settingsPagePath
        Layout.fillWidth: true

        onLoaded: {
            item.model = Qt.binding(() => (exportModel))
            item.firstColumnWidth = Qt.binding(() => (privateProperties.firstColumnWidth))
        }
    }

    RadioButtonGroup {
        Layout.fillWidth: true
        visible: model.length > 1
        spacing: 12
        orientation: Qt.Vertical

        model: exportModel.availableUnitTypes

        delegate: RoundedRadioButton {
            text: modelData["text"]
            width: parent.width
            checked: exportModel.selectedUnitType === modelData["value"]
            onToggled: {
                exportModel.selectedUnitType = modelData["value"]
            }
        }
    }
}
