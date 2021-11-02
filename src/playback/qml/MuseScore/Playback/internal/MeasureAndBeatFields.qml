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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

RowLayout {
    id: root

    property alias measureNumber: measureNumberField.value
    property alias maxMeasureNumber: measureNumberField.maxValue

    property alias beatNumber: beatNumberField.value
    property alias maxBeatNumber: beatNumberField.maxValue

    property var font: ui.theme.largeBodyFont

    signal measureNumberEdited(var newValue)
    signal beatNumberEdited(var newValue)

    spacing: 4

    NumberInputField {
        id: measureNumberField

        minValue: 1
        addLeadingZeros: false

        font: root.font

        onValueEdited: function(newValue) {
            root.measureNumberEdited(newValue)
        }
    }

    StyledTextLabel {
        text: "."
        font: root.font
    }

    NumberInputField {
        id: beatNumberField

        minValue: 1
        addLeadingZeros: false

        font: root.font

        onValueEdited: function(newValue) {
            root.beatNumberEdited(newValue)
        }
    }
}
