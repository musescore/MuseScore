/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

import MuseScore.Playback

Item {
    id: root

    required property PracticeEmphasisPanelModel emphasisModel
    property NavigationSection navigationSection: null
    property int tabNavigationOrder: 1
    property NavigationPanel navigationPanel: null
    property int navigationOrderStart: 1
    property bool showTitle: true

    readonly property bool narrow: width > 0 && width < 480
    readonly property int partCount: root.emphasisModel.parts ? root.emphasisModel.parts.length : 0
    readonly property int partNavStart: root.navigationOrderStart + 10
    readonly property int editingNavStart: root.partNavStart + Math.max(root.partCount, 1)

    property bool syncingEditingFields: false

    function showCheckBoxTooltip(item, title, description) {
        if (item.hovered) {
            ui.tooltip.show(item, title, description)
        } else {
            ui.tooltip.hide(item)
        }
    }

    function hideCheckBoxTooltipOnPress(item) {
        if (item.pressed) {
            ui.tooltip.hide(item, true)
        }
    }

    function setDropdownIndex(dropdown, index) {
        if (!dropdown) {
            return
        }
        var next = index >= 0 ? index : 0
        if (dropdown.currentIndex !== next) {
            dropdown.currentIndex = next
        }
    }

    function setNameFieldText(name) {
        if (!nameField) {
            return
        }
        var next = name ? name : ""
        if (nameField.currentText !== next) {
            nameField.currentText = next
        }
        if (nameField.inputField && nameField.inputField.text !== next) {
            nameField.inputField.text = next
        }
    }

    function syncEditingPartFields() {
        if (root.syncingEditingFields || !root.emphasisModel) {
            return
        }
        root.syncingEditingFields = true

        if (editingDropdown) {
            root.setDropdownIndex(editingDropdown, root.emphasisModel.editingPartIndex)
        }

        if (root.emphasisModel.hasScore && !root.emphasisModel.editingPartBuiltin) {
            root.setNameFieldText(root.emphasisModel.editingPartName)
            if (parentDropdown) {
                root.setDropdownIndex(parentDropdown, root.emphasisModel.editingPartParentIndex)
            }
            if (copySourceDropdown) {
                root.setDropdownIndex(copySourceDropdown, root.emphasisModel.copySourceIndex)
            }
        }

        root.syncingEditingFields = false
    }

    Connections {
        target: root.emphasisModel
        function onEditingPartChanged() {
            Qt.callLater(root.syncEditingPartFields)
        }
        function onStateChanged() {
            Qt.callLater(root.syncEditingPartFields)
        }
    }

    Component.onCompleted: Qt.callLater(root.syncEditingPartFields)

    implicitWidth: 560
    implicitHeight: contentColumn.implicitHeight

    ColumnLayout {
        id: contentColumn

        width: root.width
        spacing: 8

        StyledTextLabel {
            Layout.fillWidth: true
            visible: root.showTitle
            text: qsTrc("playback", "Practice emphasis")
            horizontalAlignment: Text.AlignLeft
            font: ui.theme.bodyBoldFont
        }

        StyledTabBar {
            id: tabBar

            Layout.fillWidth: true
            spacing: 12

            property var currentItemNavigationIndex: []

            onCurrentItemChanged: {
                if (currentItem && currentItem.navigation) {
                    currentItemNavigationIndex = [currentItem.navigation.row, currentItem.navigation.column]
                }
            }

            NavigationPanel {
                id: tabNavPanel
                name: "PracticeEmphasisTabs"
                section: root.navigationSection
                direction: NavigationPanel.Horizontal
                order: root.tabNavigationOrder
                accessible.name: qsTrc("playback", "Practice emphasis tabs")
                enabled: tabBar.visible

                onNavigationEvent: function(event) {
                    if (event.type === NavigationEvent.AboutActive) {
                        event.setData("controlIndex", tabBar.currentItemNavigationIndex)
                    }
                }
            }

            StyledTabButton {
                fillWidth: true
                text: qsTrc("playback", "Emphasized parts")
                navigation.name: "EmphasizedPartsTab"
                navigation.panel: tabNavPanel
                navigation.column: 0
            }

            StyledTabButton {
                fillWidth: true
                text: qsTrc("playback", "Editing")
                navigation.name: "EditingTab"
                navigation.panel: tabNavPanel
                navigation.column: 1
            }

            StyledTabButton {
                fillWidth: true
                text: qsTrc("playback", "Settings")
                navigation.name: "SettingsTab"
                navigation.panel: tabNavPanel
                navigation.column: 2
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8
            visible: tabBar.currentIndex === 0

            StyledTextLabel {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignLeft
                text: qsTrc("playback", "Checked parts are loud. A shared note stays loud until every checked part that sings it is cleared.")
            }

            StyledTextLabel {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignLeft
                text: qsTrc("playback", "Closed SATB: S/A upper staff, T/B lower. 1st part = top of a chord; 2nd = notes below. One note belongs to both.")
            }

            Flow {
                Layout.fillWidth: true
                spacing: 8

                Repeater {
                    model: root.emphasisModel.parts

                    CheckBox {
                        required property var modelData
                        property int index
                        text: modelData.label
                        checked: modelData.selected
                        enabled: modelData.available
                        navigation.panel: root.navigationPanel
                        navigation.order: root.partNavStart + index
                        navigation.accessible.description: modelData.tooltip
                        onHoveredChanged: root.showCheckBoxTooltip(this, modelData.label, modelData.tooltip)
                        onPressedChanged: root.hideCheckBoxTooltipOnPress(this)
                        onClicked: root.emphasisModel.setPartSelected(modelData.id, !checked)
                    }
                }
            }
        }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: tabBar.currentIndex === 1
                onVisibleChanged: {
                    if (visible) {
                        Qt.callLater(root.syncEditingPartFields)
                    }
                }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 8
                rowSpacing: 8

                FlatButton {
                    Layout.fillWidth: true
                    text: qsTrc("playback", "Undo")
                    enabled: root.emphasisModel.canUndo
                    toolTipTitle: qsTrc("playback", "Undo")
                    toolTipDescription: qsTrc("playback", "Undo the last practice-emphasis change. This does not change the score, and score Undo does not change emphasis.")
                    navigation.panel: root.navigationPanel
                    navigation.order: root.editingNavStart
                    onClicked: root.emphasisModel.undo()
                }

                FlatButton {
                    Layout.fillWidth: true
                    text: qsTrc("playback", "Redo")
                    enabled: root.emphasisModel.canRedo
                    toolTipTitle: qsTrc("playback", "Redo")
                    toolTipDescription: qsTrc("playback", "Redo the last undone practice-emphasis change. This does not change the score.")
                    navigation.panel: root.navigationPanel
                    navigation.order: root.editingNavStart + 1
                    onClicked: root.emphasisModel.redo()
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: root.narrow ? 1 : 3
                columnSpacing: 8
                rowSpacing: 8

                StyledDropdown {
                    id: editingDropdown
                    Layout.fillWidth: true
                    enabled: root.emphasisModel.hasScore
                    model: root.emphasisModel.editingPartOptions
                    navigation.panel: root.navigationPanel
                    navigation.order: root.editingNavStart + 2
                    navigation.name: "EditingPartDropdown"
                    navigation.accessible.description: qsTrc("playback", "Clicks in highlight mode add or remove notes on this part only.")
                    onActivated: function(index, value) {
                        root.emphasisModel.editingPartId = root.emphasisModel.editingValueAt(index)
                        Qt.callLater(root.syncEditingPartFields)
                    }
                    onModelChanged: Qt.callLater(root.syncEditingPartFields)
                }

                FlatButton {
                    Layout.fillWidth: root.narrow
                    text: qsTrc("playback", "New")
                    enabled: root.emphasisModel.hasScore
                    toolTipTitle: qsTrc("playback", "New")
                    toolTipDescription: qsTrc("playback", "Create a custom part. Its parent starts as the part you were editing.")
                    navigation.panel: root.navigationPanel
                    navigation.order: root.editingNavStart + 3
                    onClicked: root.emphasisModel.addCustomPart()
                }

                FlatButton {
                    Layout.fillWidth: root.narrow
                    text: qsTrc("playback", "Delete")
                    enabled: root.emphasisModel.canDeleteEditingPart
                    toolTipTitle: qsTrc("playback", "Delete")
                    toolTipDescription: qsTrc("playback", "Delete this custom part. Built-in SATB parts cannot be deleted.")
                    navigation.panel: root.navigationPanel
                    navigation.order: root.editingNavStart + 4
                    onClicked: root.emphasisModel.deleteEditingPart()
                }
            }

            ColumnLayout {
                id: customPartEditors
                Layout.fillWidth: true
                spacing: 8
                visible: root.emphasisModel.hasScore && !root.emphasisModel.editingPartBuiltin
                onVisibleChanged: {
                    if (visible) {
                        root.syncEditingPartFields()
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.narrow ? 1 : 2
                    columnSpacing: 8
                    rowSpacing: 8

                    StyledTextLabel {
                        text: qsTrc("playback", "Name")
                    }

                    TextInputField {
                        id: nameField
                        Layout.fillWidth: true
                        navigation.panel: root.navigationPanel
                        navigation.order: root.editingNavStart + 5
                        onTextChanged: function(newTextValue) {
                            if (root.syncingEditingFields) {
                                return
                            }
                            root.emphasisModel.applyEditingPartName(root.emphasisModel.editingPartId, newTextValue)
                        }
                        onTextEditingFinished: function(newTextValue) {
                            if (root.syncingEditingFields) {
                                return
                            }
                            root.emphasisModel.applyEditingPartName(root.emphasisModel.editingPartId, newTextValue)
                        }
                    }

                    StyledTextLabel {
                        text: qsTrc("playback", "Parent")
                    }

                    StyledDropdown {
                        id: parentDropdown
                        Layout.fillWidth: true
                        model: root.emphasisModel.parentOptions
                        navigation.panel: root.navigationPanel
                        navigation.order: root.editingNavStart + 6
                        navigation.name: "ParentPartDropdown"
                        navigation.accessible.description: qsTrc("playback", "Notes this part inherits before its own added and removed clicks.")
                        onModelChanged: Qt.callLater(root.syncEditingPartFields)
                        onActivated: function(index, value) {
                            if (root.syncingEditingFields) {
                                return
                            }
                            root.emphasisModel.applyEditingPartParentId(root.emphasisModel.editingPartId,
                                                                        root.emphasisModel.parentValueAt(index))
                            Qt.callLater(root.syncEditingPartFields)
                        }
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.narrow ? 1 : 3
                    columnSpacing: 8
                    rowSpacing: 8

                    StyledTextLabel {
                        text: qsTrc("playback", "Copy from")
                    }

                    StyledDropdown {
                        id: copySourceDropdown
                        Layout.fillWidth: true
                        enabled: root.emphasisModel.canCopyFromPart
                        model: root.emphasisModel.copySourceOptions
                        navigation.panel: root.navigationPanel
                        navigation.order: root.editingNavStart + 7
                        navigation.name: "CopySourceDropdown"
                        navigation.accessible.description: qsTrc("playback", "Choose the part whose current notes will be copied onto this custom part.")
                        onModelChanged: Qt.callLater(root.syncEditingPartFields)
                        onActivated: function(index, value) {
                            if (root.syncingEditingFields) {
                                return
                            }
                            root.emphasisModel.copySourceId = value
                            Qt.callLater(root.syncEditingPartFields)
                        }
                    }

                    FlatButton {
                        Layout.fillWidth: root.narrow
                        text: qsTrc("playback", "Copy")
                        enabled: root.emphasisModel.canCopyFromPart
                        toolTipTitle: qsTrc("playback", "Copy")
                        toolTipDescription: qsTrc("playback", "Copy another part’s current notes onto this custom part as its own added and removed notes, relative to its parent or to none. One undoable step.")
                        navigation.panel: root.navigationPanel
                        navigation.order: root.editingNavStart + 8
                        onClicked: root.emphasisModel.copyFromPart()
                    }
                }
            }

            FlatButton {
                id: resetButton
                Layout.fillWidth: true
                Layout.minimumWidth: implicitWidth
                text: qsTrc("playback", "Reset this part’s edits")
                enabled: root.emphasisModel.hasScore
                toolTipTitle: qsTrc("playback", "Reset this part’s edits")
                toolTipDescription: qsTrc("playback", "Remove only this part’s added and removed clicks. Notes from the rule or parent come back.")
                navigation.panel: root.navigationPanel
                navigation.order: root.editingNavStart + 9
                onClicked: root.emphasisModel.resetEditingPartEdits()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8
            visible: tabBar.currentIndex === 2

            CheckBox {
                text: qsTrc("playback", "Use for playback and export")
                checked: root.emphasisModel.playbackActive
                enabled: root.emphasisModel.hasScore
                navigation.panel: root.navigationPanel
                navigation.order: root.navigationOrderStart
                navigation.accessible.description: qsTrc("playback", "Use this mix when playing the score and when exporting audio.")
                onHoveredChanged: root.showCheckBoxTooltip(this, qsTrc("playback", "Use for playback and export"),
                                                           navigation.accessible.description)
                onPressedChanged: root.hideCheckBoxTooltipOnPress(this)
                onClicked: root.emphasisModel.playbackActive = !checked
            }

            CheckBox {
                text: qsTrc("playback", "Highlight notes and click to edit")
                checked: root.emphasisModel.emphasisMode
                enabled: root.emphasisModel.hasScore
                navigation.panel: root.navigationPanel
                navigation.order: root.navigationOrderStart + 1
                navigation.accessible.description: qsTrc("playback", "Show the edited part in red, a pale highlight behind notes you added, and faded orange for other loud notes. Click a note to add or remove it on that part only.")
                onHoveredChanged: root.showCheckBoxTooltip(this, qsTrc("playback", "Highlight notes and click to edit"),
                                                           navigation.accessible.description)
                onPressedChanged: root.hideCheckBoxTooltipOnPress(this)
                onClicked: root.emphasisModel.emphasisMode = !checked
            }

            CheckBox {
                text: qsTrc("playback", "Solo")
                checked: root.emphasisModel.solo
                enabled: root.emphasisModel.hasScore
                navigation.panel: root.navigationPanel
                navigation.order: root.navigationOrderStart + 2
                navigation.accessible.description: qsTrc("playback", "Playback and export use only the part being edited. Other notes are muted. Checked parts stay as they are.")
                onHoveredChanged: root.showCheckBoxTooltip(this, qsTrc("playback", "Solo"),
                                                           navigation.accessible.description)
                onPressedChanged: root.hideCheckBoxTooltipOnPress(this)
                onClicked: root.emphasisModel.solo = !checked
            }

            GridLayout {
                Layout.fillWidth: true
                columns: root.narrow ? 2 : 4
                columnSpacing: 12
                rowSpacing: 8

                StyledTextLabel {
                    text: qsTrc("playback", "Emphasized")
                }

                IncrementalPropertyControl {
                    Layout.preferredWidth: 88
                    currentValue: root.emphasisModel.emphasizedDb
                    minValue: -24
                    maxValue: 24
                    step: 1
                    decimals: 0
                    measureUnitsSymbol: qsTrc("playback", "dB")
                    enabled: root.emphasisModel.hasScore
                    navigation.panel: root.navigationPanel
                    navigation.order: root.navigationOrderStart + 3
                    navigation.accessible.description: qsTrc("playback", "Gain in decibels for notes that belong to a checked part.")
                    onValueEdited: function(newValue) {
                        root.emphasisModel.emphasizedDb = newValue
                    }
                }

                StyledTextLabel {
                    text: qsTrc("playback", "Others")
                }

                IncrementalPropertyControl {
                    Layout.preferredWidth: 88
                    currentValue: root.emphasisModel.otherDb
                    minValue: -40
                    maxValue: 12
                    step: 1
                    decimals: 0
                    measureUnitsSymbol: qsTrc("playback", "dB")
                    enabled: root.emphasisModel.hasScore
                    navigation.panel: root.navigationPanel
                    navigation.order: root.navigationOrderStart + 4
                    navigation.accessible.description: qsTrc("playback", "Gain in decibels for notes that do not belong to any checked part.")
                    onValueEdited: function(newValue) {
                        root.emphasisModel.otherDb = newValue
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: root.narrow ? 2 : 6
                columnSpacing: 12
                rowSpacing: 8

                StyledTextLabel {
                    text: qsTrc("playback", "Measures")
                }

                IncrementalPropertyControl {
                    Layout.preferredWidth: 72
                    currentValue: root.emphasisModel.fromMeasure
                    minValue: 0
                    maxValue: 999
                    step: 1
                    decimals: 0
                    enabled: root.emphasisModel.hasScore
                    navigation.panel: root.navigationPanel
                    navigation.order: root.navigationOrderStart + 5
                    navigation.accessible.description: qsTrc("playback", "First measure of the emphasis range. 0 is the whole score.")
                    onValueEdited: function(newValue) {
                        root.emphasisModel.fromMeasure = newValue
                    }
                }

                StyledTextLabel {
                    text: qsTrc("playback", "to")
                }

                IncrementalPropertyControl {
                    Layout.preferredWidth: 72
                    currentValue: root.emphasisModel.toMeasure
                    minValue: 0
                    maxValue: 999
                    step: 1
                    decimals: 0
                    enabled: root.emphasisModel.hasScore
                    navigation.panel: root.navigationPanel
                    navigation.order: root.navigationOrderStart + 6
                    navigation.accessible.description: qsTrc("playback", "Last measure of the emphasis range. 0 is the whole score.")
                    onValueEdited: function(newValue) {
                        root.emphasisModel.toMeasure = newValue
                    }
                }

                FlatButton {
                    Layout.fillWidth: true
                    text: root.emphasisModel.selectionButtonText
                    enabled: root.emphasisModel.hasScore
                    toolTipTitle: qsTrc("playback", "From Selection")
                    toolTipDescription: qsTrc("playback", "Set the measure range from the current score selection.")
                    navigation.panel: root.navigationPanel
                    navigation.order: root.navigationOrderStart + 7
                    onClicked: root.emphasisModel.useSelectionRange()
                }

                FlatButton {
                    Layout.fillWidth: true
                    text: qsTrc("playback", "All")
                    enabled: root.emphasisModel.hasScore
                    toolTipTitle: qsTrc("playback", "All")
                    toolTipDescription: qsTrc("playback", "Clear the measure range so emphasis uses the whole score.")
                    navigation.panel: root.navigationPanel
                    navigation.order: root.navigationOrderStart + 8
                    onClicked: root.emphasisModel.clearMeasureRange()
                }
            }

            StyledTextLabel {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignLeft
                text: qsTrc("playback", "0 is the whole score. Highlight is red for the part being edited; a pale mark sits behind notes you added. Other loud notes are faded.")
            }
        }
    }
}
