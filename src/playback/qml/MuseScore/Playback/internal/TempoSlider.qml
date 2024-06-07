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
import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.UiComponents 1.0
import Muse.Ui 1.0

RowLayout {
    id: root

    property real value: 0
    property real from: 0.1
    property real to: 3
    property real stepSize: 0.05

    signal moved(real newValue)

    height: 30
    spacing: 0

    StyledTextLabel {
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignVCenter

        text: qsTrc("playback", "Tempo")
        font: ui.theme.largeBodyFont
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    NumberInputField {
        value: Math.round(root.value * 100)
        minValue: root.from * 100
        maxValue: root.to * 100

        live: false

        addLeadingZeros: false
        font: ui.theme.largeBodyFont

        onValueEdited: function(newValue) {
            root.moved(newValue / 100)
        }
    }

    StyledTextLabel {
        text: "%"
        font: ui.theme.largeBodyFont
    }

    StyledSlider {
        id: slider

        Layout.preferredWidth: root.width / 2
        Layout.leftMargin: 12

        value: root.value
        from: root.from
        to: root.to
        stepSize: root.stepSize

        fillBackground: false

        onMoved: {
            root.moved(slider.value)
        }
    }
}
