/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
import MuseScore.Playback 1.0

Column {
    id: root

    property NavigationPanel navigationPanel: null

    height: implicitHeight

    spacing: 8

    VSTParamsModel {
        id: vstParamsModel
    }

    Component.onCompleted: {
        vstParamsModel.init()
    }

    Row {
        spacing: 2

        StyledTextLabel {
            text: qsTrc("playback", "Keyswitch")
        }

        StyledTextLabel {
            text: qsTrc("playback", "(MIDI note number)")
            opacity: 0.7
        }
    }

    IncrementalPropertyControl {
        id: valueControl

        width: 68

        currentValue: vstParamsModel.keySwitchStr
        hint: vstParamsModel.defaultKeySwitchStr()

        minValue: 0
        maxValue: 127

        navigation.panel: root.navigationPanel
        navigation.row: 1

        canIncrease: vstParamsModel.keySwitch < maxValue
        onIncrement: function() {
            return vstParamsModel.increaseKeySwitch(currentValue)
        }

        canDecrease: vstParamsModel.keySwitch > minValue
        onDecrement: function() {
            return vstParamsModel.decreaseKeySwitch(currentValue)
        }

        onValueEditingFinished: function(newValue) {
            vstParamsModel.keySwitchStr = newValue
        }
    }
}
