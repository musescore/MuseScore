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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

StyledDialogView {
    id: root

    contentWidth: 600
    contentHeight: mainColumn.implicitHeight + mainColumn.spacing + buttonBox.implicitHeight
    margins: 16

    title: qsTrc("notation", "Transpose")

    // TransposeDialogModel.mode values (kept in sync with engraving/dom/mscore.h TransposeMode)
    readonly property int modeToKey: 0
    readonly property int modeByInterval: 1
    readonly property int modeDiatonic: 2

    readonly property int dirUp: 0
    readonly property int dirDown: 1
    readonly property int dirClosest: 2

    TransposeDialogModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    NavigationPanel {
        id: navPanel
        name: "TransposeDialog"
        section: root.navigationSection
        order: 1
    }

    ColumnLayout {
        id: mainColumn

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        spacing: 16

        RowLayout {
            id: modesRow

            Layout.fillWidth: true
            spacing: 16

            StyledGroupBox {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                title: qsTrc("notation", "Transpose chromatically")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            text: qsTrc("notation", "To Key")
                        }

                        StyledDropdown {
                            id: keyList
                            Layout.fillWidth: true

                            enabled: model.enableTransposeToKey

                            navigation.name: "KeyList"
                            navigation.panel: navPanel
                            navigation.row: 0

                            model: [
                                { text: qsTrc("notation", "C♭ major / A♭ minor") },
                                { text: qsTrc("notation", "G♭ major / E♭ minor") },
                                { text: qsTrc("notation", "D♭ major / B♭ minor") },
                                { text: qsTrc("notation", "A♭ major / F minor") },
                                { text: qsTrc("notation", "E♭ major / C minor") },
                                { text: qsTrc("notation", "B♭ major / G minor") },
                                { text: qsTrc("notation", "F major / D minor") },
                                { text: qsTrc("notation", "C major / A minor") },
                                { text: qsTrc("notation", "G major / E minor") },
                                { text: qsTrc("notation", "D major / B minor") },
                                { text: qsTrc("notation", "A major / F♯ minor") },
                                { text: qsTrc("notation", "E major / C♯ minor") },
                                { text: qsTrc("notation", "B major / G♯ minor") },
                                { text: qsTrc("notation", "F♯ major / D♯ minor") },
                                { text: qsTrc("notation", "C♯ major / A♯ minor") }
                            ]

                            currentIndex: model.keyIndex

                            onActivated: function(index, value) {
                                model.keyIndex = index
                                model.mode = root.modeToKey
                            }
                        }

                        RadioButtonGroup {
                            id: keyDirection
                            Layout.fillWidth: true

                            orientation: ListView.Horizontal
                            spacing: 12

                            model: [
                                { text: qsTrc("notation", "Closest"), value: root.dirClosest },
                                { text: qsTrc("notation", "Up"), value: root.dirUp },
                                { text: qsTrc("notation", "Down"), value: root.dirDown }
                            ]

                            delegate: RoundedRadioButton {
                                required property var modelData
                                required property int index

                                text: modelData.text
                                checked: model.mode === root.modeToKey && model.direction === modelData.value

                                navigation.name: "KeyDirection" + index
                                navigation.panel: navPanel
                                navigation.row: 1 + index

                                onToggled: {
                                    model.mode = root.modeToKey
                                    model.direction = modelData.value
                                }
                            }
                        }
                    }

                    SeparatorLine {}

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            text: qsTrc("notation", "By interval")
                        }

                        StyledDropdown {
                            id: intervalList
                            Layout.fillWidth: true

                            navigation.name: "IntervalList"
                            navigation.panel: navPanel
                            navigation.row: 4

                            model: [
                                { text: qsTrc("notation", "Perfect unison") },
                                { text: qsTrc("notation", "Augmented unison") },
                                { text: qsTrc("notation", "Diminished second") },
                                { text: qsTrc("notation", "Minor second") },
                                { text: qsTrc("notation", "Major second") },
                                { text: qsTrc("notation", "Augmented second") },
                                { text: qsTrc("notation", "Diminished third") },
                                { text: qsTrc("notation", "Minor third") },
                                { text: qsTrc("notation", "Major third") },
                                { text: qsTrc("notation", "Augmented third") },
                                { text: qsTrc("notation", "Diminished fourth") },
                                { text: qsTrc("notation", "Perfect fourth") },
                                { text: qsTrc("notation", "Augmented fourth") },
                                { text: qsTrc("notation", "Diminished fifth") },
                                { text: qsTrc("notation", "Perfect fifth") },
                                { text: qsTrc("notation", "Augmented fifth") },
                                { text: qsTrc("notation", "Diminished sixth") },
                                { text: qsTrc("notation", "Minor sixth") },
                                { text: qsTrc("notation", "Major sixth") },
                                { text: qsTrc("notation", "Augmented sixth") },
                                { text: qsTrc("notation", "Diminished seventh") },
                                { text: qsTrc("notation", "Minor seventh") },
                                { text: qsTrc("notation", "Major seventh") },
                                { text: qsTrc("notation", "Augmented seventh") },
                                { text: qsTrc("notation", "Diminished octave") },
                                { text: qsTrc("notation", "Perfect octave") }
                            ]

                            currentIndex: model.intervalIndex

                            onActivated: function(index, value) {
                                model.intervalIndex = index
                                model.mode = root.modeByInterval
                            }
                        }

                        RadioButtonGroup {
                            id: intervalDirection
                            Layout.fillWidth: true

                            orientation: ListView.Horizontal
                            spacing: 12

                            model: [
                                { text: qsTrc("notation", "Up"), value: root.dirUp },
                                { text: qsTrc("notation", "Down"), value: root.dirDown }
                            ]

                            delegate: RoundedRadioButton {
                                required property var modelData
                                required property int index

                                text: modelData.text
                                checked: model.mode === root.modeByInterval && model.direction === modelData.value

                                navigation.name: "IntervalDirection" + index
                                navigation.panel: navPanel
                                navigation.row: 5 + index

                                onToggled: {
                                    model.mode = root.modeByInterval
                                    model.direction = modelData.value
                                }
                            }
                        }
                    }

                    CheckBox {
                        id: transposeKeysCheckBox
                        Layout.fillWidth: true

                        enabled: model.enableTransposeKeys && model.mode !== root.modeDiatonic

                        navigation.panel: navPanel
                        navigation.row: 7
                        navigation.accessible.name: text

                        text: qsTrc("notation", "Transpose key signatures")
                        checked: model.transposeKeys

                        onClicked: model.transposeKeys = !model.transposeKeys
                    }
                }
            }

            StyledGroupBox {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                title: qsTrc("notation", "Transpose diatonically")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    StyledDropdown {
                        id: degreeList
                        Layout.fillWidth: true

                        navigation.name: "DegreeList"
                        navigation.panel: navPanel
                        navigation.row: 8

                        model: [
                            { text: qsTrc("notation", "Second") },
                            { text: qsTrc("notation", "Third") },
                            { text: qsTrc("notation", "Fourth") },
                            { text: qsTrc("notation", "Fifth") },
                            { text: qsTrc("notation", "Sixth") },
                            { text: qsTrc("notation", "Seventh") }
                        ]

                        currentIndex: model.degreeIndex

                        onActivated: function(index, value) {
                            model.degreeIndex = index
                            model.mode = root.modeDiatonic
                        }
                    }

                    RadioButtonGroup {
                        id: degreeDirection
                        Layout.fillWidth: true

                        orientation: ListView.Horizontal
                        spacing: 12

                        model: [
                            { text: qsTrc("notation", "Up"), value: root.dirUp },
                            { text: qsTrc("notation", "Down"), value: root.dirDown }
                        ]

                        delegate: RoundedRadioButton {
                            required property var modelData
                            required property int index

                            text: modelData.text
                            checked: model.mode === root.modeDiatonic && model.direction === modelData.value

                            navigation.name: "DegreeDirection" + index
                            navigation.panel: navPanel
                            navigation.row: 9 + index

                            onToggled: {
                                model.mode = root.modeDiatonic
                                model.direction = modelData.value
                            }
                        }
                    }

                    CheckBox {
                        id: keepDegreeAlterationsCheckBox
                        Layout.fillWidth: true
                        enabled: model.mode === root.modeDiatonic
                        navigation.panel: navPanel
                        navigation.row: 11
                        navigation.accessible.name: text

                        text: qsTrc("notation", "Keep degree alterations")
                        checked: model.keepDegreeAlterations

                        onClicked: model.keepDegreeAlterations = !model.keepDegreeAlterations
                    }

                    // spacer to visually balance the shorter diatonic box against the chromatic box
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            title: qsTrc("notation", "Options")

            ColumnLayout {
                anchors.fill: parent
                spacing: 12

                CheckBox {
                    id: transposeChordNamesCheckBox
                    Layout.fillWidth: true

                    enabled: model.enableTransposeChordNames

                    navigation.panel: navPanel
                    navigation.row: 12
                    navigation.accessible.name: text

                    text: qsTrc("notation", "Transpose chord symbols and fretboard diagrams")
                    checked: model.transposeChordNames

                    onClicked: model.transposeChordNames = !model.transposeChordNames
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation", "Accidentals:")
                    }

                    StyledDropdown {
                        id: accidentalOptions
                        Layout.fillWidth: true

                        navigation.name: "AccidentalOptions"
                        navigation.panel: navPanel
                        navigation.row: 13

                        model: [
                            { text: qsTrc("notation", "Single ♯ and ♭ only"), value: false },
                            { text: qsTrc("notation", "Use double ♯ and ♭"), value: true }
                        ]

                        currentIndex: model.useDoubleSharpsFlats ? 1 : 0

                        onActivated: function(index, value) {
                            model.useDoubleSharpsFlats = value
                        }
                    }
                }
            }
        }
    }

    ButtonBox {
        id: buttonBox

        anchors {
            top: mainColumn.bottom
            topMargin: mainColumn.spacing
            right: parent.right
        }

        navigationPanel.section: root.navigationSection
        navigationPanel.order: navPanel.order + 1

        buttons: [ ButtonBoxModel.Cancel, ButtonBoxModel.Ok ]

        onStandardButtonClicked: function(buttonId) {
            if (buttonId === ButtonBoxModel.Cancel) {
                root.reject()
            } else if (buttonId === ButtonBoxModel.Ok) {
                model.apply()
                root.hide()
            }
        }
    }
}
