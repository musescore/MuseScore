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
import QtQuick.Layouts 1.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

ColumnLayout{
    id: column

    required property StyleItem styleItem
    required property string imageON
    required property string imageOFF

    property alias text: toggleText.text

    RowLayout {
        spacing: 15

        StyledImage {
            forceWidth: 100
            forceHeight: 80
            horizontalPadding: 12
            verticalPadding: 10

            source: toggleButton.checked ? imageON : imageOFF
        }

        RowLayout {
            id: toggle

            ToggleButton {
                id: toggleButton
                checked: styleItem.value === true
                onToggled: {
                    styleItem.value = !styleItem.value
                }
            }

            StyledTextLabel {
                id : toggleText
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignLeft
            }
        }
    }

}
