/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project

Column {
    id: root

    required property ExportDialogModel exportModel
    property alias navigation: navPanel

    readonly property bool isExportAvailable: Boolean(pageLoader.item)
                                              ? pageLoader.item.isExportAvailable
                                              : true

    spacing: 12

    Component.onCompleted: {
        pageLoader.refresh()
    }

    NavigationPanel {
        id: navPanel
        name: "ExportOptions"
        enabled: root.visible && root.enabled
        direction: NavigationPanel.Vertical
    }

    ExportOptionItem {
        id: typeLabel
        width: parent.width
        text: qsTrc("project/export", "Format:")

        StyledDropdown {
            id: typeDropdown
            Layout.fillWidth: true

            navigation.name: "ExportTypeDropdown"
            navigation.panel: navPanel
            navigation.row: 1
            navigation.accessible.name: typeLabel.text + " " + currentText

            model: root.exportModel.exportTypeList()
            popupItemsCount: typeDropdown.count

            textRole: "name"
            valueRole: "id"

            currentIndex: {
                // First, check if it's a subtype
                const index = model.findIndex(function(type) {
                    return type.subtypes.some(function(subtype) {
                        return subtype.id === root.exportModel.selectedExportType.id
                    })
                })

                if (index !== -1) {
                    return index
                }

                // Otherwise, it must be a toplevel type
                return typeDropdown.indexOfValue(root.exportModel.selectedExportType.id)
            }

            onActivated: function(index, value) {
                root.exportModel.selectExportTypeById(value)
            }
        }
    }

    ExportOptionItem {
        id: subtypeLabel
        width: parent.width
        visible: subtypeComboBox.count > 0
        text: qsTrc("project/export", "File type:")

        StyledDropdown {
            id: subtypeComboBox
            Layout.fillWidth: true

            navigation.name: "ExportSubtypeDropdown"
            navigation.panel: navPanel
            navigation.row: 2
            navigation.accessible.name: subtypeLabel.text + " " + currentText

            model: {
                if (typeDropdown.currentIndex > -1) {
                    return typeDropdown.model[typeDropdown.currentIndex].subtypes
                }

                return []
            }

            textRole: "name"
            valueRole: "id"

            currentIndex: subtypeComboBox.indexOfValue(root.exportModel.selectedExportType.id)

            onActivated: function(index, value) {
                root.exportModel.selectExportTypeById(value)
            }
        }
    }

    Loader {
        id: pageLoader
        width: parent.width
        visible: status === Loader.Ready

        function refresh() {
            if (!root.exportModel.selectedExportType.settingsPagePath) {
                setSource("")
            }

            var properties = {
                model: Qt.binding(() => root.exportModel),
                navigationPanel: navPanel,
                navigationOrder: 3
            }

            setSource(root.exportModel.selectedExportType.settingsPagePath, properties)
        }

        Connections {
            target: root.exportModel

            function onSelectedExportTypeChanged() {
                pageLoader.refresh()
            }
        }
    }

    RadioButtonGroup {
        id: exportType

        width: parent.width
        visible: count > 1
        spacing: 12
        orientation: Qt.Vertical

        model: root.exportModel.availableUnitTypes

        delegate: RoundedRadioButton {
            width: ListView.view.width

            required text
            required property int value
            required property int index

            navigation.name: "ExportType_" + text
            navigation.panel: navPanel
            navigation.row: 100000 + index

            checked: root.exportModel.selectedUnitType === value
            onToggled: {
                root.exportModel.selectedUnitType = value
            }
        }
    }

    CheckBox {
        id: selectedBarsBox
        width: parent.width
        enabled: root.exportModel.canExportSelectedBars

        text: {
            if (!root.exportModel.canExportSelectedBars) {
                return qsTrc("project/export", "Export selected bars only")
            }
            if (root.exportModel.selectedBarStart === root.exportModel.selectedBarEnd) {
                return qsTrc("project/export", "Export selected bars only (bar %1)").arg(root.exportModel.selectedBarStart)
            }
            return qsTrc("project/export", "Export selected bars only (bars %1–%2)")
                    .arg(root.exportModel.selectedBarStart)
                    .arg(root.exportModel.selectedBarEnd)
        }

        navigation.name: "ExportSelectedBarsCheckbox"
        navigation.panel: navPanel
        navigation.row: 100000 + exportType.count

        checked: root.exportModel.exportSelectedBars
        onClicked: {
            root.exportModel.exportSelectedBars = !checked
        }
    }

    StyledTextLabel {
        width: parent.width
        visible: !root.exportModel.canExportSelectedBars
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignLeft
        text: qsTrc("project/export", "Select a range of bars in the score to export just those bars.")
    }

    ExportOptionItem {
        width: parent.width
        visible: selectedBarsBox.checked
        text: qsTrc("project/export", "Repetitions:")

        IncrementalPropertyControl {
            Layout.preferredWidth: 96

            currentValue: root.exportModel.selectedBarsRepeats
            minValue: 1
            maxValue: 99
            step: 1
            decimals: 0

            navigation.name: "SelectedBarsRepeats"
            navigation.panel: navPanel
            navigation.row: 100000 + exportType.count + 1

            onValueEdited: function(newValue) {
                root.exportModel.selectedBarsRepeats = newValue
            }
        }
    }

    SeparatorLine {
        anchors.topMargin: 24
        anchors.bottomMargin: 24

        visible: root.isExportAvailable
    }

    CheckBox {
        width: parent.width
        text: qsTrc("project/export", "Open destination folder on export")

        visible: root.isExportAvailable

        navigation.name: "OpenDestinationFolderOnExportCheckbox"
        navigation.panel: navPanel
        navigation.row: 100000 + exportType.count + 2

        checked: root.exportModel.shouldDestinationFolderBeOpenedOnExport
        onClicked: {
            root.exportModel.shouldDestinationFolderBeOpenedOnExport = !checked
        }
    }
}
