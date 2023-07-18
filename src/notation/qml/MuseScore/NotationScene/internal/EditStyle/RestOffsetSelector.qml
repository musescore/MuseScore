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

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    anchors.fill: parent
    color: ui.theme.backgroundPrimaryColor

    RestsPageModel {
        id: restsPageModel
    }

    RadioButtonGroup {
        model: [
            { text: qsTrc("notation", "1 space"), value: false },
            { text: qsTrc("notation", "2 spaces"), value: true }
        ]

        delegate: FlatRadioButton {
            width: 106
            height: 30

            checked: modelData.value === restsPageModel.multiVoiceRestTwoSpaceOffset.value
            text: modelData.text

            onToggled: {
                restsPageModel.multiVoiceRestTwoSpaceOffset.value = modelData.value
            }
        }
    }
}
