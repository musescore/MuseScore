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

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Rectangle {
    anchors.fill: parent
    color: ui.theme.backgroundPrimaryColor

    BendStyleSelector {
        id: bendStyleSelector
    }

    RadioButtonGroup {
        model: [
            { iconCode: IconCode.GUITAR_BEND_STYLE_1, text: "", value: false },
            { iconCode: IconCode.GUITAR_BEND_STYLE_FULL, text: "", value: true }
        ]

        delegate: FlatRadioButton {
            width: 106
            height: 60

            checked: modelData.value === bendStyleSelector.useFull.value

            StyledIconLabel {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                iconCode: modelData.iconCode
                font.pixelSize: 28
            }

            onToggled: {
                bendStyleSelector.useFull.value = modelData.value
            }
        }
    }
}
