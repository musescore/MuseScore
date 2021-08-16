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
import QtQuick 2.9

import MuseScore.UiComponents 1.0

Row {
    id: root

    property int numerator: 0
    property int denominator: 0
    property var availableDenominators: null

    signal numeratorSelected(var value)
    signal denominatorSelected(var value)

    spacing: 14

    IncrementalPropertyControl {
        id: control

        implicitWidth: 68
        anchors.verticalCenter: parent.verticalCenter

        currentValue: root.numerator
        step: 1
        decimals: 0
        maxValue: 63
        minValue: 1

        onValueEdited: {
            root.numeratorSelected(newValue)
        }
    }

    StyledTextLabel {
        anchors.verticalCenter: parent.verticalCenter
        font: ui.theme.largeBodyFont
        text: "/"
    }

    Dropdown {
        id: timeComboBox

        width: control.width
        anchors.verticalCenter: parent.verticalCenter

        popupItemsCount: 4
        currentIndex: timeComboBox.indexOfValue(root.denominator)

        model: {
            var resultList = []

            var denominators = root.availableDenominators
            for (var i = 0; i < denominators.length; ++i) {
                resultList.push({"text" : denominators[i], "value" : denominators[i]})
            }

            return resultList
        }

        onCurrentValueChanged: {
            root.denominatorSelected(timeComboBox.currentValue)
        }
    }
}
