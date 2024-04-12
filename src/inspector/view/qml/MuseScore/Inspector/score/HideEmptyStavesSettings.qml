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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0

    width: parent.width

    spacing: 12

    function focusOnFirst() {
         dontHideEmptyStavesInFirstSystem.navigation.requestActive()
     }

    CheckBox {
        id: dontHideEmptyStavesInFirstSystem
        width: parent.width

        navigation.name: "DontHideEmptyStavesInFirstSystem"
        navigation.panel: root.navigationPanel
        navigation.order: root.navigationRowStart

        text: qsTrc("inspector", "Don’t hide empty staves in first system")
        checked: root.model ? root.model.dontHideEmptyStavesInFirstSystem : false

        onClicked: {
            if (root.model) {
                root.model.dontHideEmptyStavesInFirstSystem = !checked
            }
        }
    }

    CheckBox {
        width: parent.width

        navigation.name: "ShowBracketsWhenSpanningSingleStaff"
        navigation.panel: root.navigationPanel
        navigation.order: root.navigationRowStart + 1

        text: qsTrc("inspector", "Show brackets when spanning a single staff")
        checked: root.model ? root.model.showBracketsWhenSpanningSingleStaff : false

        onClicked: {
            if (root.model) {
                root.model.showBracketsWhenSpanningSingleStaff = !checked
            }
        }
    }
}
