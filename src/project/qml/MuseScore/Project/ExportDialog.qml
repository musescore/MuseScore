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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project

import "internal/Export"

StyledDialogView {
    id: root

    property var params: null
    readonly property bool selectionMode: Boolean(params && params["selectionMode"])

    title: selectionMode
           ? qsTrc("project/export", "Export selection")
           : qsTrc("project/export", "Export")

    contentWidth: selectionMode ? 680 : 756
    contentHeight: selectionMode ? 620 : 420
    margins: 24

    ExportDialogModel {
        id: exportModel
        selectionMode: root.selectionMode
    }

    onNavigationActivateRequested: {
        exportScoresListView.focusOnFirst()
    }

    onClosed: {
        exportModel.updateExportInfo()
    }

    RowLayout {
        anchors.fill: parent
        spacing: 2 * root.margins

        ColumnLayout {
            visible: !root.selectionMode
            Layout.fillWidth: true
            Layout.preferredWidth: (parent.width - parent.spacing) / 2
            spacing: 18

            StyledTextLabel {
                text: qsTrc("project/export", "Select parts to export")
                font: ui.theme.bodyBoldFont
            }

            ExportScoresListView {
                id: exportScoresListView
                Layout.fillWidth: true
                Layout.fillHeight: true

                scoresModel: exportModel
                navigation.section: root.navigationSection
                navigation.order: 1
            }

            RowLayout {
                spacing: 12

                NavigationPanel {
                    id: leftButtonsNavPanel
                    name: "Export dialog left buttons"
                    section: root.navigationSection
                    order: 2
                    direction: NavigationPanel.Horizontal
                }

                FlatButton {
                    Layout.fillWidth: true

                    text: qsTrc("project/export", "Select all")

                    navigation.name: "Select all"
                    navigation.panel: leftButtonsNavPanel
                    navigation.column: 1

                    onClicked: {
                        exportModel.setAllSelected(true)
                    }
                }

                FlatButton {
                    Layout.fillWidth: true

                    text: qsTrc("project/export", "Clear selection")

                    navigation.name: "Clear selection"
                    navigation.panel: leftButtonsNavPanel
                    navigation.order: 2

                    onClicked: {
                        exportModel.setAllSelected(false)
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width / 2 - parent.spacing / 2
            spacing: root.selectionMode ? 12 : 18

            StyledTextLabel {
                text: qsTrc("project/export", "Export settings")
                font: ui.theme.bodyBoldFont
            }

            StyledTextLabel {
                visible: root.selectionMode
                Layout.fillWidth: true
                text: qsTrc("project/export", "Only the selected range will be exported.")
                wrapMode: Text.WordWrap
            }

            RowLayout {
                visible: root.selectionMode
                Layout.fillWidth: true
                spacing: 24

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    StyledTextLabel {
                        text: qsTrc("project/export", "Tempo:")
                        horizontalAlignment: Text.AlignLeft
                    }

                    IncrementalPropertyControl {
                        Layout.preferredWidth: 76
                        currentValue: exportModel.selectionTempoPercentage
                        minValue: 10
                        maxValue: 300
                        step: 5
                        decimals: 0
                        measureUnitsSymbol: "%"

                        navigation.name: "SelectionExportTempo"
                        navigation.panel: exportOptionsView.navigation
                        navigation.row: 1
                        navigation.accessible.name: qsTrc("project/export", "Export tempo")

                        onValueEdited: function(newValue) {
                            exportModel.selectionTempoPercentage = newValue
                        }
                    }

                    StyledSlider {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 30

                        value: exportModel.selectionTempoPercentage
                        from: 10
                        to: 300
                        stepSize: 5

                        fillBackground: false

                        onMoved: {
                            exportModel.selectionTempoPercentage = Math.round(value)
                        }
                    }
                }

                CheckBox {
                    text: qsTrc("project/export", "Metronome")
                    checked: exportModel.selectionMetronomeEnabled

                    navigation.name: "SelectionExportMetronome"
                    navigation.panel: exportOptionsView.navigation
                    navigation.row: 2
                    navigation.accessible.name: qsTrc("project/export", "Export metronome")

                    onClicked: {
                        exportModel.selectionMetronomeEnabled = !checked
                    }
                }
            }

            RowLayout {
                visible: root.selectionMode
                Layout.fillWidth: true
                spacing: 12

                CheckBox {
                    Layout.fillWidth: true
                    enabled: !exportModel.selectionMetronomeEnabled
                    text: qsTrc("project/export", "Fade in")
                    checked: exportModel.selectionFadeInEnabled

                    navigation.name: "SelectionExportFadeIn"
                    navigation.panel: exportOptionsView.navigation
                    navigation.row: 3

                    onClicked: {
                        exportModel.selectionFadeInEnabled = !checked
                    }
                }

                IncrementalPropertyControl {
                    Layout.preferredWidth: 92
                    enabled: !exportModel.selectionMetronomeEnabled
                             && exportModel.selectionFadeInEnabled
                    currentValue: exportModel.selectionFadeInDuration
                    minValue: 0.1
                    maxValue: 30
                    step: 0.5
                    decimals: 1
                    measureUnitsSymbol: qsTrc("global", "s")

                    navigation.name: "SelectionExportFadeInDuration"
                    navigation.panel: exportOptionsView.navigation
                    navigation.row: 4
                    navigation.accessible.name: qsTrc("project/export", "Fade-in duration")

                    onValueEdited: function(newValue) {
                        exportModel.selectionFadeInDuration = newValue
                    }
                }

                CheckBox {
                    Layout.fillWidth: true
                    text: qsTrc("project/export", "Fade out")
                    checked: exportModel.selectionFadeOutEnabled

                    navigation.name: "SelectionExportFadeOut"
                    navigation.panel: exportOptionsView.navigation
                    navigation.row: 5

                    onClicked: {
                        exportModel.selectionFadeOutEnabled = !checked
                    }
                }

                IncrementalPropertyControl {
                    Layout.preferredWidth: 92
                    enabled: exportModel.selectionFadeOutEnabled
                    currentValue: exportModel.selectionFadeOutDuration
                    minValue: 0.1
                    maxValue: 30
                    step: 0.5
                    decimals: 1
                    measureUnitsSymbol: qsTrc("global", "s")

                    navigation.name: "SelectionExportFadeOutDuration"
                    navigation.panel: exportOptionsView.navigation
                    navigation.row: 6
                    navigation.accessible.name: qsTrc("project/export", "Fade-out duration")

                    onValueEdited: function(newValue) {
                        exportModel.selectionFadeOutDuration = newValue
                    }
                }
            }

            RowLayout {
                visible: root.selectionMode && exportModel.hasOtherInstruments
                Layout.fillWidth: true
                spacing: 12

                CheckBox {
                    text: qsTrc("project/export", "Mute other instruments")
                    checked: exportModel.selectionOtherInstrumentsMuted

                    navigation.name: "SelectionExportMuteOtherInstruments"
                    navigation.panel: exportOptionsView.navigation
                    navigation.row: 7

                    onClicked: {
                        exportModel.selectionOtherInstrumentsMuted = !checked
                    }
                }

                StyledTextLabel {
                    text: qsTrc("project/export", "Other instruments:")
                    horizontalAlignment: Text.AlignLeft
                }

                IncrementalPropertyControl {
                    Layout.preferredWidth: 76
                    enabled: !exportModel.selectionOtherInstrumentsMuted
                    currentValue: exportModel.selectionOtherInstrumentsVolume
                    minValue: 0
                    maxValue: 100
                    step: 5
                    decimals: 0
                    measureUnitsSymbol: "%"

                    navigation.name: "SelectionExportOtherInstrumentsVolume"
                    navigation.panel: exportOptionsView.navigation
                    navigation.row: 8
                    navigation.accessible.name: qsTrc("project/export", "Other instruments volume")

                    onValueEdited: function(newValue) {
                        exportModel.selectionOtherInstrumentsVolume = newValue
                    }
                }

                StyledSlider {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    enabled: !exportModel.selectionOtherInstrumentsMuted

                    value: exportModel.selectionOtherInstrumentsVolume
                    from: 0
                    to: 100
                    stepSize: 5

                    fillBackground: false

                    onMoved: {
                        exportModel.selectionOtherInstrumentsVolume = Math.round(value)
                    }
                }
            }

            StyledTextLabel {
                visible: root.selectionMode
                Layout.fillWidth: true
                text: qsTrc("project/export", "Selected instrument levels")
                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            StyledListView {
                id: selectedInstrumentLevelsView

                visible: root.selectionMode
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(contentHeight, 104)
                spacing: 6

                model: exportModel.selectionInstruments.filter(function(instrument) {
                    return instrument.selected
                })

                delegate: instrumentLevelDelegate
            }

            ExpandableBlank {
                visible: root.selectionMode && exportModel.hasOtherInstruments
                Layout.fillWidth: true

                title: qsTrc("project/export", "Individual other-instrument levels")
                isExpanded: false

                navigation.name: "SelectionExportOtherInstrumentLevels"
                navigation.panel: exportOptionsView.navigation
                navigation.row: 9

                contentItemComponent: StyledListView {
                    implicitHeight: Math.min(contentHeight, 168)
                    width: parent.width
                    spacing: 6

                    model: exportModel.selectionInstruments.filter(function(instrument) {
                        return !instrument.selected
                    })

                    delegate: instrumentLevelDelegate
                }
            }

            ExportOptionsView {
                id: exportOptionsView

                Layout.fillHeight: true
                Layout.fillWidth: true

                exportModel: exportModel
                navigation.section: root.navigationSection
                navigation.order: 3
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 12

                ButtonBox {
                    Layout.fillWidth: true

                    buttons: [ ButtonBoxModel.Cancel ]

                    navigationPanel.section: root.navigationSection
                    navigationPanel.order: 4

                    FlatButton {
                        text: qsTrc("project/export", "Export…")
                        buttonRole: ButtonBoxModel.AcceptRole
                        buttonId: ButtonBoxModel.Done
                        enabled: exportModel.selectionLength > 0 && exportOptionsView.isExportAvailable
                        accentButton: true

                        onClicked: {
                            if (exportModel.exportScores()) {
                                root.hide();
                            }
                        }
                    }

                    onStandardButtonClicked: function(buttonId) {
                        if (buttonId === ButtonBoxModel.Cancel) {
                            root.hide()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: instrumentLevelDelegate

        RowLayout {
            id: instrumentRow

            required property var modelData
            property int editedVolume: modelData.volume

            width: ListView.view ? ListView.view.width : 0
            height: 32
            spacing: 12

            readonly property bool controlEnabled: modelData.selected
                                                   || !exportModel.selectionOtherInstrumentsMuted

            StyledTextLabel {
                Layout.preferredWidth: 150
                Layout.maximumWidth: 150
                text: modelData.name
                font: modelData.selected ? ui.theme.bodyBoldFont : ui.theme.bodyFont
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideRight
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 76
                enabled: instrumentRow.controlEnabled
                currentValue: instrumentRow.editedVolume
                minValue: 0
                maxValue: 200
                step: 5
                decimals: 0
                measureUnitsSymbol: "%"

                navigation.name: "SelectionExportInstrumentVolume" + modelData.index
                navigation.panel: exportOptionsView.navigation
                navigation.row: 20 + modelData.index
                navigation.accessible.name: qsTrc("project/export", "%1 volume").arg(modelData.name)

                onValueEdited: function(newValue) {
                    instrumentRow.editedVolume = newValue
                    exportModel.setSelectionInstrumentVolume(modelData.index, newValue)
                }
            }

            StyledSlider {
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                enabled: instrumentRow.controlEnabled

                value: instrumentRow.editedVolume
                from: 0
                to: 200
                stepSize: 5

                fillBackground: false

                onMoved: {
                    instrumentRow.editedVolume = Math.round(value)
                }

                onPressedChanged: {
                    if (!pressed) {
                        exportModel.setSelectionInstrumentVolume(modelData.index, instrumentRow.editedVolume)
                    }
                }
            }
        }
    }
}
