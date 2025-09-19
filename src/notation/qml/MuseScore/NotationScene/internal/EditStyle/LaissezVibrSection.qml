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
    title: qsTrc("notation/editstyle/slursandties", "Laissez vibrer")

    ColumnLayout {
        spacing: 12

        GridLayout {
            columns: 3
            columnSpacing: 8
            rowSpacing: 8

            StyledTextLabel {
                text: qsTrc("notation/editstyle/slursandties", "Minimum laissez vibrer length:")
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 80
                currentValue: root.pageModel.minLaissezVibLength.value

                minValue: 0.0
                maxValue: 99.99
                step: 0.1
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                onValueEdited: newValue => root.pageModel.minLaissezVibLength.value = newValue
            }

            StyleResetButton {
                styleItem: root.pageModel.minLaissezVibLength
            }
        }

        CheckBox {
            Layout.fillWidth: true
            text: qsTrc("notation/editstyle/slursandties", "Use SMuFL symbols")
            checked: root.pageModel.laissezVibUseSmuflSym.value
            onClicked: root.pageModel.laissezVibUseSmuflSym.value = !checked
        }
    }
}
