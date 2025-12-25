/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

StyleControlRowWithReset {
    id: root

    property string suffix: ''
    property bool inPercentage: false
    property double step: 0.01
    property double spinBoxWidth: 80
    property alias min: spinBox.minValue
    property alias max: spinBox.maxValue
    property alias decimals: spinBox.decimals

    IncrementalPropertyControl {
        id: spinBox

        width: root.spinBoxWidth

        currentValue: root.inPercentage ? Math.round(root.styleItem.value * 100) : root.styleItem.value
        minValue: 0
        maxValue: root.inPercentage ? 999 : 99
        step: root.inPercentage ? 1 : root.step
        decimals: root.inPercentage ? 0 : 2
        measureUnitsSymbol: root.inPercentage ? '%' : root.suffix

        onValueEdited: function(newValue) {
            root.styleItem.value = root.inPercentage ? newValue / 100 : newValue
        }
    }
}
