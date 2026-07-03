/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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
import MuseScore.PropertiesPanel

FlatRadioButtonGroupPropertyView {
    id: root

    property int orientation : Qt.Vertical

    titleText: qsTrc("propertiespanel", "Direction")

    navigationName: "Direction"

    states: [
        State {
            name: "VERTICAL"
            when: root.orientation === Qt.Vertical

            PropertyChanges {
                target: root
                model: [
                    { text: qsTrc("propertiespanel", "Auto"), value: DirectionTypes.VERTICAL_AUTO, title: qsTrc("propertiespanel", "Auto", "direction") },
                    { iconCode: IconCode.ARROW_DOWN, value: DirectionTypes.VERTICAL_DOWN, title: qsTrc("propertiespanel", "Down", "direction") },
                    { iconCode: IconCode.ARROW_UP, value: DirectionTypes.VERTICAL_UP, title: qsTrc("propertiespanel", "Up", "direction") }
                ]
            }
        },

        State {
            name: "HORIZONTAL"
            when: root.orientation === Qt.Horizontal

            PropertyChanges {
                target: root
                model: [
                   { text: qsTrc("propertiespanel", "Auto"), value: DirectionTypes.HORIZONTAL_AUTO, title: qsTrc("propertiespanel", "Auto", "direction") },
                   { iconCode: IconCode.ARROW_LEFT, value: DirectionTypes.HORIZONTAL_LEFT, title: qsTrc("propertiespanel", "Left", "direction") },
                   { iconCode: IconCode.ARROW_RIGHT, value: DirectionTypes.HORIZONTAL_RIGHT, title: qsTrc("propertiespanel", "Right", "direction") }
                ]
            }
        }
    ]
}
