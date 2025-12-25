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
    title: qsTrc("notation/editstyle/slursandties", "Slurs")

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
                currentValue: root.pageModel.slurEndWidth.value

                minValue: 0.0
                maxValue: 99.99
                step: 0.05
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                onValueEdited: newValue => root.pageModel.slurEndWidth.value = newValue
            }

            StyleResetButton {
                styleItem: root.pageModel ? root.pageModel.slurEndWidth : null
            }

            StyledTextLabel {
                text: qsTrc("notation/editstyle/slursandties", "Line thickness middle:")
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 80
                currentValue: root.pageModel.slurMidWidth.value

                minValue: 0.0
                maxValue: 99.99
                step: 0.05
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                onValueEdited: newValue => root.pageModel.slurMidWidth.value = newValue
            }

            StyleResetButton {
                styleItem: root.pageModel.slurMidWidth
            }

            StyledTextLabel {
                text: qsTrc("notation/editstyle/slursandties", "Dotted line thickness:")
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 80
                currentValue: root.pageModel.slurDottedWidth.value

                minValue: 0.0
                maxValue: 99.99
                step: 0.05
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                onValueEdited: newValue => root.pageModel.slurDottedWidth.value = newValue
            }

            StyleResetButton {
                styleItem: root.pageModel.slurDottedWidth
            }

            StyledTextLabel {
                text: qsTrc("notation/editstyle/slursandties", "Autoplace min. distance:")
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 80
                currentValue: root.pageModel.slurMinDistance.value

                minValue: 0.0
                maxValue: 99.99
                step: 0.5
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                onValueEdited: newValue => root.pageModel.slurMinDistance.value = newValue
            }

            StyleResetButton {
                styleItem: root.pageModel.slurMinDistance
            }
        }

        ColumnLayout {
            spacing: 8

            StyledTextLabel {
                Layout.fillWidth: true
                text: qsTrc("notation/editstyle/slursandties", "Partial slurs across repeats and breaks:")
                horizontalAlignment: Text.AlignLeft
            }

            ButtonGroup {
                id: partialSlurAngleGroup
            }

            Repeater {
                model: [
                    { text: qsTrc("notation/editstyle/slursandties", "Follow contour of notes"), value: false },
                    { text: qsTrc("notation/editstyle/slursandties", "Angle away from staff"), value: true }
                ]
                delegate: RoundedRadioButton {
                    ButtonGroup.group: partialSlurAngleGroup
                    text: modelData.text
                    checked: root.pageModel.angleHangingSlursAwayFromStaff.value === modelData.value
                    onToggled: root.pageModel.angleHangingSlursAwayFromStaff.value = modelData.value
                }
            }
        }
    }
}
