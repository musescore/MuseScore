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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

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
            required iconCode
            required property bool value
            
            width: 106
            height: 60

            iconFontSize: 28

            checked: value === bendStyleSelector.useFull.value
            onToggled: {
                bendStyleSelector.useFull.value = value
            }
        }
    }
}
