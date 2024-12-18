/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.UiComponents

Column {
    id: root

    property alias title: label.text
    property alias value: spinBox.currentValue
    property alias measureUnit: spinBox.measureUnitsSymbol
    property alias incrementStep: spinBox.step
    property alias minValue: spinBox.minValue
    property alias maxValue: spinBox.maxValue

    property alias navigation: spinBox.navigation

    signal valueEdited(real newValue)

    width: {
        if (!parent) {
            return 0
        }

        if (!(parent instanceof Grid)) {
            return parent.width
        }

        const grid = parent as Grid
        return (grid.width - grid.spacing * (grid.columns - 1)) / grid.columns
    }

    spacing: 8

    StyledTextLabel {
        id: label
        width: parent.width
        horizontalAlignment: Text.AlignLeft
    }

    IncrementalPropertyControl {
        id: spinBox
        navigation.accessible.name: root.title + " " + currentValue

        onValueEdited: function (newValue) {
            root.valueEdited(newValue)
        }
    }
}
