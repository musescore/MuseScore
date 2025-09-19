/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
import QtQuick.Controls
import QtQuick.Layouts

import MuseScore.NotationScene
import Muse.UiComponents
import Muse.Ui

StyledGroupBox {
    id: root

    required property var pageModel

    Layout.fillWidth: true
    title: qsTrc("notation/editstyle/slursandties", "Ties")

    ColumnLayout {
        spacing: 12

        GridLayout {
            columns: 3
            columnSpacing: 8
            rowSpacing: 8

            StyledTextLabel {
                text: qsTrc("notation/editstyle/slursandties", "Line thickness at end:")
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 80
                currentValue: root.pageModel.tieEndWidth.value

                minValue: 0.0
                maxValue: 99.99
                step: 0.05
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                onValueEdited: newValue => root.pageModel.tieEndWidth.value = newValue
            }

            StyleResetButton {
                styleItem: root.pageModel.tieEndWidth
            }

            StyledTextLabel {
                text: qsTrc("notation/editstyle/slursandties", "Line thickness middle:")
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 80
                currentValue: root.pageModel.tieMidWidth.value

                minValue: 0.0
                maxValue: 99.99
                step: 0.05
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                onValueEdited: newValue => root.pageModel.tieMidWidth.value = newValue
            }

            StyleResetButton {
                styleItem: root.pageModel.tieMidWidth
            }

            StyledTextLabel {
                text: qsTrc("notation/editstyle/slursandties", "Dotted line thickness:")
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 80
                currentValue: root.pageModel.tieDottedWidth.value

                minValue: 0.0
                maxValue: 99.99
                step: 0.05
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                onValueEdited: newValue => root.pageModel.tieDottedWidth.value = newValue
            }

            StyleResetButton {
                styleItem: root.pageModel.tieDottedWidth
            }

            StyledTextLabel {
                text: qsTrc("notation/editstyle/slursandties", "Autoplace min. distance:")
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 80
                currentValue: root.pageModel.tieMinDistance.value

                minValue: 0.0
                maxValue: 99.99
                step: 0.5
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                onValueEdited: newValue => root.pageModel.tieMinDistance.value = newValue
            }

            StyleResetButton {
                styleItem: root.pageModel.tieMinDistance
            }

            StyledTextLabel {
                text: qsTrc("notation/editstyle/slursandties", "Minimum tie length:")
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 80
                currentValue: root.pageModel.minTieLength.value

                minValue: 0.0
                maxValue: 99.99
                step: 0.1
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                onValueEdited: newValue => root.pageModel.minTieLength.value = newValue
            }

            StyleResetButton {
                styleItem: root.pageModel.minTieLength
            }

            StyledTextLabel {
                text: qsTrc("notation/editstyle/slursandties", "Minimum hanging tie length:")
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 80
                currentValue: root.pageModel.minHangingTieLength.value

                minValue: 0.0
                maxValue: 99.99
                step: 0.1
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                onValueEdited: newValue => root.pageModel.minHangingTieLength.value = newValue
            }

            StyleResetButton {
                styleItem: root.pageModel.minHangingTieLength
            }
        }

        ColumnLayout {
            spacing: 8

            StyledTextLabel {
                Layout.fillWidth: true
                text: qsTrc("notation", "Placement on single notes:")
                horizontalAlignment: Text.AlignLeft
            }

            RadioButtonGroup {
                height: 30
                model: [
                    { iconCode: IconCode.TIE_INSIDE, value: 1, title: qsTrc("inspector", "Inside") },
                    { iconCode: IconCode.TIE_OUTSIDE, value: 2, title: qsTrc("inspector", "Outside") }
                ]

                delegate: FlatRadioButton {
                    width: 106
                    height: 30

                    checked: modelData.value === root.pageModel.tiePlacementSingleNote.value
                    iconCode: modelData.iconCode
                    navigation.accessible.name: modelData.title

                    onToggled: root.pageModel.tiePlacementSingleNote.value = modelData.value
                }
            }
        }

        ColumnLayout {
            spacing: 8

            StyledTextLabel {
                Layout.fillWidth: true
                text: qsTrc("notation", "Placement on chords:")
                horizontalAlignment: Text.AlignLeft
            }

            RadioButtonGroup {
                height: 30
                model: [
                    { iconCode: IconCode.TIE_CHORD_INSIDE, value: 1, title: qsTrc("inspector", "Inside") },
                    { iconCode: IconCode.TIE_CHORD_OUTSIDE, value: 2, title: qsTrc("inspector", "Outside") }
                ]

                delegate: FlatRadioButton {
                    width: 106
                    height: 30

                    checked: modelData.value === root.pageModel.tiePlacementChord.value
                    iconCode: modelData.iconCode
                    navigation.accessible.name: modelData.title

                    onToggled: root.pageModel.tiePlacementChord.value = modelData.value
                }
            }
        }

        ColumnLayout {
            spacing: 8

            StyledTextLabel {
                Layout.fillWidth: true
                text: qsTrc("notation", "Placement of inner ties with respect to augmentation dots:")
                horizontalAlignment: Text.AlignLeft
            }

            RoundedRadioButton {
                Layout.fillWidth: true
                checked: root.pageModel.tieDotsPlacement.value === 0
                onToggled: root.pageModel.tieDotsPlacement.value = 0
                text: qsTrc("notation", "Auto")
            }

            RoundedRadioButton {
                Layout.fillWidth: true
                checked: root.pageModel.tieDotsPlacement.value === 1
                onToggled: root.pageModel.tieDotsPlacement.value = 1
                text: qsTrc("notation", "Always before dots")
            }

            RoundedRadioButton {
                Layout.fillWidth: true
                checked: root.pageModel.tieDotsPlacement.value === 2
                onToggled: root.pageModel.tieDotsPlacement.value = 2
                text: qsTrc("notation", "Always after dots")
            }
        }
    }
}
