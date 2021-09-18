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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

InspectorPropertyView {
    id: root

    property alias spinBox: spinBoxItem

    property alias minValue: spinBoxItem.minValue
    property alias maxValue: spinBoxItem.maxValue
    property alias step: spinBoxItem.step
    property alias decimals: spinBoxItem.decimals
    property alias measureUnitsSymbol: spinBoxItem.measureUnitsSymbol
    property alias icon: spinBoxItem.icon
    property alias iconMode: spinBoxItem.iconMode

    IncrementalPropertyControl {
        id: spinBoxItem

        navigation.name: root.navigation.name + " Value"
        navigation.panel: root.navigation.panel
        navigation.column: root.navigation.column
        navigation.row: root.navigation.row + 1

        isIndeterminate: root.propertyItem ? root.propertyItem.isUndefined : true
        currentValue: root.propertyItem ? root.propertyItem.value : 0

        onValueEdited: function(newValue) {
            if (root.propertyItem) {
                root.propertyItem.value = newValue
            }
        }
    }
}
