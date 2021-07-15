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
import QtQuick 2.9
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: InspectorPropertyView {
        width: root.width

        navigation.name: "Glissando Style Menu"
        navigation.panel: root.navigation.panel
        navigation.column: root.navigation.column
        navigation.row: root.navigation.row + 1

        titleText: qsTrc("inspector", "Style")
        propertyItem: root.model ? root.model.styleType : null

        Dropdown {
            id: gstyles

            width: parent.width

            navigation.name: "Glissando Style Value"
            navigation.panel: root.navigation.panel
            navigation.column: root.navigation.column
            navigation.row: root.navigation.row + 2

            model: [
                { text: "Chromatic", value: Glissando.STYLE_CHROMATIC },
                { text: "White keys", value: Glissando.STYLE_WHITE_KEYS },
                { text: "Black keys", value: Glissando.STYLE_BLACK_KEYS },
                { text: "Diatonic", value: Glissando.STYLE_DIATONIC },
                { text: "Portamento", value: Glissando.STYLE_PORTAENTO }
            ]

            currentIndex: root.model && !root.model.styleType.isUndefined ? gstyles.indexOfValue(root.model.styleType.value) : -1

            onCurrentValueChanged: {
                if (currentIndex === -1) {
                    return
                }

                root.model.styleType.value = gstyles.currentValue
            }
        }
    }
}
