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
import QtQuick.Layouts

import Muse.UiComponents
import Muse.Ui

import MuseScore.Playback

RowLayout {
    id: root

    property PlaybackToolBarModel playbackModel: null

    property NavigationPanel navigationPanel: null
    property int navigationOrderStart: 0

    spacing: 12

    StyledTextLabel {
        Layout.fillWidth: true
        Layout.fillHeight: true

        text: qsTrc("playback", "Speed")
        font: ui.theme.largeBodyFont
        horizontalAlignment: Text.AlignLeft
    }

    IncrementalPropertyControl {
        Layout.preferredWidth: 76
        currentValue: (root.playbackModel.tempoMultiplier * 100).toFixed(decimals)

        maxValue: 300
        minValue: 10
        step: 5
        measureUnitsSymbol: "%"
        decimals: 0

        navigation.panel: root.navigationPanel
        navigation.order: root.navigationOrderStart
        navigation.accessible.name: qsTrc("playback", "Speed")

        onValueEdited: function(newValue) {
            root.playbackModel.tempoMultiplier = newValue / 100
        }
    }

    StyledSlider {
        id: slider

        Layout.preferredWidth: root.width / 2

        value: root.playbackModel.tempoMultiplier
        from: 0.1
        to: 3.0
        stepSize: 0.05

        fillBackground: false

        onMoved: {
            root.playbackModel.tempoMultiplier = value
        }
    }
}
