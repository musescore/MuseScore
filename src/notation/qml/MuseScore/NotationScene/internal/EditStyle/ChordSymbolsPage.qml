/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
import QtQuick.Layouts 1.15
import QtQuick.Controls

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

StyledFlickable {
    id: root

    contentWidth: column.width
    contentHeight: column.height

    ChordSymbolsPageModel {
        id: chordSymbolsModel
    }

    property var extensionStyles: [
        chordSymbolsModel.extensionMag,
        chordSymbolsModel.extensionAdjust
    ];

    property var modifierStyles: [
        chordSymbolsModel.modifierMag,
        chordSymbolsModel.modifierAdjust,
        chordSymbolsModel.verticallyStackModifiers
    ];

    property var bassStyles: [
        chordSymbolsModel.chordBassNoteStagger,
        chordSymbolsModel.chordBassNoteScale
    ];

    property var polychordStyles: [
        chordSymbolsModel.polychordDividerThickness,
        chordSymbolsModel.polychordDividerSpacing
    ];

    property var alignmentStyles: [
        chordSymbolsModel.verticallyAlignChordSymbols,
        chordSymbolsModel.chordAlignmentToNotehead,
        chordSymbolsModel.chordAlignmentToFretboard,
        chordSymbolsModel.chordAlignmentExcludeModifiers
    ];

    property var capitalizationStyles: [
        chordSymbolsModel.automaticCapitalization,
        chordSymbolsModel.lowerCaseMinorChords,
        chordSymbolsModel.lowerCaseBassNotes,
        chordSymbolsModel.allCapsNoteNames
    ];

    property var positioningStyles: [
        chordSymbolsModel.harmonyFretDist,
        chordSymbolsModel.minHarmonyDist
    ];

    property var playbackStyles: [
        chordSymbolsModel.harmonyVoiceLiteral,
        chordSymbolsModel.harmonyVoicing,
        chordSymbolsModel.harmonyDuration,
        chordSymbolsModel.capoPosition
    ];

    function resetStyles(styles) {
        for (let styleItem of styles) {
            styleItem.value = styleItem.defaultValue
        }
    }

    function isResetEnabled(styles) {
        for (const styleItem of styles) {
            if (!styleItem.isDefault) {
                return true;
            }
        }
        return false;
    }

    signal goToTextStylePage(string s)

    ColumnLayout {
        id: column
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/chordsymbols", "Preset")

            ColumnLayout {
                spacing: 12
                anchors.fill: parent

                // TODO - replace with StyledDropdown once this whole dialog is written in QML
                ComboBoxDropdown {
                    id: presetDropdown
                    Layout.preferredWidth: 191
                    model: chordSymbolsModel.possiblePresetOptions()

                    styleItem: chordSymbolsModel.chordStylePreset

                    onHandleItem: function(value) {
                        chordSymbolsModel.setChordStyle(value)
                    }
                }

                RowLayout {
                    enabled: chordSymbolsModel.isCustomXml
                    visible: chordSymbolsModel.isCustomXml
                    height: 30

                    TextInputField {
                        id: customXMLField
                        Layout.preferredWidth: 191
                        readOnly: true
                        currentText: chordSymbolsModel.chordDescriptionFile ? chordSymbolsModel.chordDescriptionFile.value : ""
                    }

                    FlatButton {
                        icon: IconCode.OPEN_FILE
                        onClicked: chordSymbolsModel.selectChordDescriptionFile()
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/chordsymbols", "Appearance")

            ColumnLayout {
                spacing: 12
                anchors.fill: parent

                FlatButton {
                    text: qsTrc("notation", "Edit chord symbol text style")

                    onClicked: {
                        root.goToTextStylePage("chord-symbols")
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/chordsymbols", "Spelling")

                    RowLayout {
                        spacing: 6
                        anchors.fill: parent

                        RadioButtonGroup {
                            id: spellingRadioButtonGroup
                            Layout.fillWidth: true
                            model: chordSymbolsModel.possibleChordSymbolSpellings()

                            delegate: RoundedRadioButton {
                                height: 30
                                checked: modelData.value === chordSymbolsModel.chordSymbolSpelling.value
                                text: modelData.text ? modelData.text : ""

                                onToggled: {
                                    chordSymbolsModel.chordSymbolSpelling.value = modelData.value
                                }
                            }
                        }

                        FlatButton {
                            Layout.alignment: Qt.AlignTop | Qt.AlignRight
                            icon: IconCode.UNDO
                            enabled: !chordSymbolsModel.chordSymbolSpelling.isDefault
                            onClicked: chordSymbolsModel.chordSymbolSpelling.value = chordSymbolsModel.chordSymbolSpelling.defaultValue
                        }
                    }
                }

                StyledGroupBox {

                    label: CheckBox {
                        id: capitaliseCheckBox
                        text: qsTrc("notation/editstyle/chordsymbols", "Automatically capitalize note names")

                        checked: chordSymbolsModel.automaticCapitalization && Boolean(chordSymbolsModel.automaticCapitalization.value)
                        onClicked: {
                            if (chordSymbolsModel.automaticCapitalization) {
                                chordSymbolsModel.automaticCapitalization.value = !checked
                            }
                        }
                    }

                    Layout.fillWidth: true

                    RowLayout {
                        spacing: 6
                        anchors.fill: parent

                        CheckBox {
                            text: qsTrc("notation/editstyle/chordsymbols", "Lowercase minor chords")
                            checked: chordSymbolsModel.lowerCaseMinorChords.value === true
                            onClicked: chordSymbolsModel.lowerCaseMinorChords.value = !chordSymbolsModel.lowerCaseMinorChords.value
                        }

                        CheckBox {
                            text: qsTrc("notation/editstyle/chordsymbols", "Lowercase bass notes")
                            checked: chordSymbolsModel.lowerCaseBassNotes.value === true
                            onClicked: chordSymbolsModel.lowerCaseBassNotes.value = !chordSymbolsModel.lowerCaseBassNotes.value
                        }

                        CheckBox {
                            text: qsTrc("notation/editstyle/chordsymbols", "All caps note names")
                            checked: chordSymbolsModel.allCapsNoteNames.value === true
                            onClicked: chordSymbolsModel.allCapsNoteNames.value = !chordSymbolsModel.allCapsNoteNames.value
                        }

                        FlatButton {
                            Layout.alignment: Qt.AlignTop | Qt.AlignRight
                            icon: IconCode.UNDO
                            enabled: isResetEnabled(root.capitalizationStyles)
                            onClicked: resetStyles(root.capitalizationStyles)
                        }
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/chordsymbols", "Extensions (e.g. 7, 11)")

                    ColumnLayout {
                        spacing: 12
                        anchors.fill: parent

                        RowLayout {
                            spacing: 6
                            Layout.fillWidth: true

                            StyledTextLabel {
                                Layout.preferredWidth: 120
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                text: qsTrc("notation", "Scale:")
                            }

                            Item {
                                Layout.preferredWidth: 326
                                implicitHeight: children.length === 1 ? children[0].implicitHeight : 0
                                IncrementalPropertyControl {
                                    width: 80

                                    currentValue: Math.round(chordSymbolsModel.extensionMag.value * 100)
                                    minValue: 0
                                    maxValue: 999
                                    step: 1
                                    decimals: 0

                                    measureUnitsSymbol: '%'

                                    onValueEdited: function(newValue) {
                                        chordSymbolsModel.extensionMag.value = newValue / 100
                                    }
                                }
                            }

                            FlatButton {
                                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                                icon: IconCode.UNDO
                                enabled: isResetEnabled(root.extensionStyles)
                                onClicked: resetStyles(root.extensionStyles)
                            }
                        }

                        RowLayout {
                            spacing: 6
                            Layout.fillWidth: true

                            StyledTextLabel {
                                Layout.preferredWidth: 120
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                text: qsTrc("notation", "Vertical offset:")
                            }

                            IncrementalPropertyControl {
                                Layout.preferredWidth: 80

                                currentValue: Math.round(chordSymbolsModel.extensionAdjust.value * 100)
                                minValue: -999
                                maxValue: 999
                                step: 1
                                decimals: 0

                                measureUnitsSymbol: '%'

                                onValueEdited: function(newValue) {
                                    chordSymbolsModel.extensionAdjust.value = newValue / 100
                                }
                            }
                        }
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/chordsymbols", "Modifiers (e.g. sus4, no 3)")

                    ColumnLayout {
                        spacing: 12
                        anchors.fill: parent

                        RowLayout {
                            spacing: 6
                            Layout.fillWidth: true

                            StyledTextLabel {
                                Layout.preferredWidth: 120
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                text: qsTrc("notation", "Scale:")
                            }

                            Item {
                                Layout.preferredWidth: 326
                                implicitHeight: children.length === 1 ? children[0].implicitHeight : 0
                                IncrementalPropertyControl {
                                    width: 80

                                    currentValue: Math.round(chordSymbolsModel.modifierMag.value * 100)
                                    minValue: 0
                                    maxValue: 999
                                    step: 1
                                    decimals: 0

                                    measureUnitsSymbol: '%'

                                    onValueEdited: function(newValue) {
                                        chordSymbolsModel.modifierMag.value = newValue / 100
                                    }
                                }
                            }

                            FlatButton {
                                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                                icon: IconCode.UNDO
                                enabled: isResetEnabled(root.modifierStyles)
                                onClicked: resetStyles(root.modifierStyles)
                            }
                        }

                        RowLayout {
                            spacing: 6
                            Layout.fillWidth: true

                            StyledTextLabel {
                                Layout.preferredWidth: 120
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                text: qsTrc("notation", "Vertical offset:")
                            }

                            IncrementalPropertyControl {
                                Layout.preferredWidth: 80

                                currentValue: Math.round(chordSymbolsModel.modifierAdjust.value * 100)
                                minValue: -999
                                maxValue: 999
                                step: 1
                                decimals: 0

                                measureUnitsSymbol: '%'

                                onValueEdited: function(newValue) {
                                    chordSymbolsModel.modifierAdjust.value = newValue / 100
                                }
                            }
                        }

                        CheckBox {
                            text: qsTrc("notation/editstyle/chordsymbols", "Vertically stack modifiers")
                            checked: chordSymbolsModel.verticallyStackModifiers.value === true
                            onClicked: chordSymbolsModel.verticallyStackModifiers.value = !chordSymbolsModel.verticallyStackModifiers.value
                        }
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/chordsymbols", "Altered bass notes (e.g. A7/G)")

                    ColumnLayout {
                        spacing: 12
                        anchors.fill: parent

                        RowLayout {
                            spacing: 6
                            Layout.fillWidth: true

                            StyledTextLabel {
                                Layout.preferredWidth: 120
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                text: qsTrc("notation", "Style:")
                            }

                            Item {
                                Layout.preferredWidth: 326
                                implicitHeight: children.length === 1 ? children[0].implicitHeight : 0

                                RadioButtonGroup {
                                    id: radioButtonGroup

                                    enabled: !chordSymbolsModel.isLegacyXml

                                    width: 100

                                    model: [
                                        { iconCode: IconCode.CHORD_BASS_ALIGN, iconSize: 16, value: false },
                                        { iconCode: IconCode.CHORD_BASS_OFFSET, iconSize: 16, value: true }
                                    ]

                                    delegate: FlatRadioButton {
                                        height: 48

                                        navigation.accessible.name: modelData.title ? modelData.title : (modelData.text ? modelData.text : "")

                                        checked: chordSymbolsModel.chordBassNoteStagger.value === modelData.value
                                        onToggled: chordSymbolsModel.chordBassNoteStagger.value = modelData.value

                                        Column {
                                            anchors.centerIn: parent
                                            spacing: 8

                                            StyledIconLabel {
                                                anchors.horizontalCenter: parent.horizontalCenter
                                                iconCode: modelData.iconCode ? modelData.iconCode : IconCode.NONE
                                                font.pixelSize: modelData.iconSize ? modelData.iconSize : 28
                                            }

                                            StyledTextLabel {
                                                anchors.horizontalCenter: parent.horizontalCenter
                                                text: modelData.text ? modelData.text : ""
                                            }
                                        }
                                    }
                                }
                            }

                            FlatButton {
                                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                                icon: IconCode.UNDO
                                enabled: isResetEnabled(root.bassStyles)
                                onClicked: resetStyles(root.bassStyles)
                            }
                        }

                        RowLayout {
                            spacing: 6
                            Layout.fillWidth: true

                            StyledTextLabel {
                                Layout.preferredWidth: 120
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                text: qsTrc("notation", "Bass note scale:")
                            }

                            IncrementalPropertyControl {
                                Layout.preferredWidth: 80

                                currentValue: Math.round(chordSymbolsModel.chordBassNoteScale.value * 100)
                                minValue: 0
                                maxValue: 999
                                step: 1
                                decimals: 0

                                measureUnitsSymbol: '%'

                                onValueEdited: function(newValue) {
                                    chordSymbolsModel.chordBassNoteScale.value = newValue / 100
                                }
                            }
                        }
                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/chordsymbols", "Polychords (e.g. C|F♯)")

                    ColumnLayout {
                        spacing: 12
                        anchors.fill: parent

                        RowLayout {
                            spacing: 6
                            Layout.fillWidth: true

                            StyledTextLabel {
                                Layout.preferredWidth: 120
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                text: qsTrc("notation", "Divider thickness:")
                            }

                            Item {
                                Layout.preferredWidth: 326
                                implicitHeight: children.length === 1 ? children[0].implicitHeight : 0
                                IncrementalPropertyControl {
                                    width: 80

                                    currentValue: chordSymbolsModel.polychordDividerThickness.value
                                    minValue: 0
                                    maxValue: 99
                                    step: 0.01
                                    decimals: 2

                                    measureUnitsSymbol: qsTrc("global", "sp")

                                    onValueEdited: function(newValue) {
                                        chordSymbolsModel.polychordDividerThickness.value = newValue
                                    }
                                }
                            }

                            FlatButton {
                                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                                icon: IconCode.UNDO
                                enabled: isResetEnabled(root.polychordStyles)
                                onClicked: resetStyles(root.polychordStyles)
                            }
                        }

                        RowLayout {
                            spacing: 6
                            Layout.fillWidth: true

                            StyledTextLabel {
                                Layout.preferredWidth: 120
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                text: qsTrc("notation", "Divider spacing:")
                            }

                            IncrementalPropertyControl {
                                Layout.preferredWidth: 80

                                currentValue: chordSymbolsModel.polychordDividerSpacing.value
                                minValue: 0
                                maxValue: 99
                                step: 0.01
                                decimals: 2

                                measureUnitsSymbol: qsTrc("global", "sp")

                                onValueEdited: function(newValue) {
                                    chordSymbolsModel.polychordDividerSpacing.value = newValue
                                }
                            }
                        }
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/chordsymbols", "Alignment")

            ColumnLayout {
                spacing: 12
                anchors.fill: parent

                RowLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    Item {
                        Layout.preferredWidth: 452
                        implicitHeight: children.length === 1 ? children[0].implicitHeight : 0

                        CheckBox {
                            text: qsTrc("notation/editstyle/chordsymbols", "Vertically align chord symbols and fretboard diagrams on the same system")
                            checked: chordSymbolsModel.verticallyAlignChordSymbols.value === true
                            onClicked: chordSymbolsModel.verticallyAlignChordSymbols.value = !chordSymbolsModel.verticallyAlignChordSymbols.value
                        }
                    }

                    FlatButton {
                        Layout.alignment: Qt.AlignTop | Qt.AlignRight
                        icon: IconCode.UNDO
                        enabled: isResetEnabled(root.alignmentStyles)
                        onClicked: resetStyles(root.alignmentStyles)
                    }
                }


                IconAndTextButtonSelector {
                    styleItem: chordSymbolsModel.chordAlignmentToNotehead
                    label: qsTrc("notation", "Alignment to notehead:")
                    controlAreaWidth: 98
                    buttonHeight: 30
                    hasReset: false

                    model: [
                        { iconCode: IconCode.NOTE_ALIGN_LEFT, iconSize: 16, value: 0},
                        { iconCode: IconCode.NOTE_ALIGN_CENTER, iconSize: 16, value: 2},
                        { iconCode: IconCode.NOTE_ALIGN_RIGHT, iconSize: 16, value: 1 }
                    ]
                }

                IconAndTextButtonSelector {
                    styleItem: chordSymbolsModel.chordAlignmentToFretboard
                    label: qsTrc("notation", "Alignment to fretboard diagram:")
                    controlAreaWidth: 98
                    buttonHeight: 30
                    hasReset: false

                    model: [
                        { iconCode: IconCode.ALIGN_LEFT, iconSize: 16, value: 0},
                        { iconCode: IconCode.ALIGN_HORIZONTAL_CENTER, iconSize: 16, value: 2},
                        { iconCode: IconCode.ALIGN_RIGHT, iconSize: 16, value: 1 }
                    ]
                }

                CheckBox {
                    text: qsTrc("notation/editstyle/chordsymbols", "Exclude modifiers from horizontal alignment")
                    checked: chordSymbolsModel.chordAlignmentExcludeModifiers.value === true
                    onClicked: chordSymbolsModel.chordAlignmentExcludeModifiers.value = !chordSymbolsModel.chordAlignmentExcludeModifiers.value
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/chordsymbols", "Positioning")

            ColumnLayout {
                spacing: 12
                anchors.fill: parent

                RowLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    StyledTextLabel {
                        Layout.preferredWidth: 120
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        text: qsTrc("notation", "Minimum space from fretboard diagram:")
                    }

                    Item {
                        Layout.preferredWidth: 326
                        implicitHeight: children.length === 1 ? children[0].implicitHeight : 0
                        IncrementalPropertyControl {
                            width: 80

                            currentValue: chordSymbolsModel.harmonyFretDist.value
                            minValue: 0
                            maxValue: 99
                            step: 0.1
                            decimals: 2

                            measureUnitsSymbol: qsTrc("global", "sp")

                            onValueEdited: function(newValue) {
                                chordSymbolsModel.harmonyFretDist.value = newValue
                            }
                        }
                    }

                    FlatButton {
                        Layout.alignment: Qt.AlignTop | Qt.AlignRight
                        icon: IconCode.UNDO
                        enabled: isResetEnabled(root.positioningStyles)
                        onClicked: resetStyles(root.positioningStyles)
                    }
                }

                RowLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    StyledTextLabel {
                        Layout.preferredWidth: 120
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        text: qsTrc("notation", "Minimum space between chord symbols:")
                    }

                    IncrementalPropertyControl {
                        Layout.preferredWidth: 80

                        currentValue: chordSymbolsModel.minHarmonyDist.value
                        minValue: 0
                        maxValue: 99
                        step: 0.1
                        decimals: 2

                        measureUnitsSymbol: qsTrc("global", "sp")

                        onValueEdited: function(newValue) {
                            chordSymbolsModel.minHarmonyDist.value = newValue
                        }
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/chordsymbols", "Playback")

            ColumnLayout {
                spacing: 12
                width: parent.width

                RowLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    StyledTextLabel {
                        Layout.preferredWidth: 120
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        text: qsTrc("notation", "Interpretation:")
                    }

                    Item {
                        Layout.preferredWidth: 326
                        Layout.preferredHeight: interpretationDropdown.height

                        // TODO - replace with StyledDropdown once this whole dialog is written in QML
                        ComboBoxDropdown {
                            id: interpretationDropdown
                            Layout.preferredWidth: 172
                            model: chordSymbolsModel.possibleHarmonyVoiceLiteralOptions()

                            styleItem: chordSymbolsModel.harmonyVoiceLiteral
                        }
                    }

                    FlatButton {
                        Layout.alignment: Qt.AlignTop | Qt.AlignRight
                        icon: IconCode.UNDO
                        enabled: isResetEnabled(root.playbackStyles)
                        onClicked: resetStyles(root.playbackStyles)
                    }
                }

                RowLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    StyledTextLabel {
                        Layout.preferredWidth: 120
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        text: qsTrc("notation", "Voicing:")
                    }

                    Item {
                        Layout.preferredWidth: 326
                        Layout.preferredHeight: voicingDropdown.height

                        // TODO - replace with StyledDropdown once this whole dialog is written in QML
                        ComboBoxDropdown {
                            id: voicingDropdown
                            Layout.preferredWidth: 172
                            model: chordSymbolsModel.possibleHarmonyVoicingOptions()

                            styleItem: chordSymbolsModel.harmonyVoicing
                        }
                    }
                }

                RowLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    StyledTextLabel {
                        Layout.preferredWidth: 120
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        text: qsTrc("notation", "Duration:")
                    }

                    Item {
                        Layout.preferredWidth: 326
                        Layout.preferredHeight: durationDropdown.height
                        // TODO - replace with StyledDropdown once this whole dialog is written in QML
                        ComboBoxDropdown {
                            id: durationDropdown
                            Layout.preferredWidth: 172
                            model: chordSymbolsModel.possibleHarmonyDurationOptions()

                            styleItem: chordSymbolsModel.harmonyDuration
                        }
                    }
                }

                RowLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    StyledTextLabel {
                        Layout.preferredWidth: 120
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        text: qsTrc("notation", "Capo fret position:")
                    }

                    IncrementalPropertyControl {
                        Layout.preferredWidth: 80

                        currentValue: chordSymbolsModel.capoPosition.value
                        minValue: 0
                        maxValue: 11
                        step: 1
                        decimals: 0

                        onValueEdited: function(newValue) {
                            chordSymbolsModel.capoPosition.value = newValue
                        }
                    }
                }
            }
        }
    }
}
