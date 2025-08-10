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

RowLayout {
    id: root
    spacing: 6

    required property StyleItem styleItem

    IncrementalPropertyControl {
        Layout.preferredWidth: 100
        decimals: 2
        measureUnitsSymbol: 'sp'
        prefixIcon: IconCode.HORIZONTAL
        minValue: -100
        maxValue: 100
        step: 0.1

        currentValue: root.styleItem.value.x
        onValueEdited: function(newValue) {
            root.styleItem.value.x = newValue
        }
    }

    IncrementalPropertyControl {
        Layout.preferredWidth: 100
        decimals: 2
        measureUnitsSymbol: 'sp'
        prefixIcon: IconCode.VERTICAL
        minValue: -100
        maxValue: 100
        step: 0.1

        currentValue: root.styleItem.value.y
        onValueEdited: function(newValue) {
            root.styleItem.value.y = newValue
        }
    }

    FlatButton {
        icon: IconCode.UNDO
        enabled: !root.styleItem.isDefault
        onClicked: root.styleItem.value = root.styleItem.defaultValue
    }
}
