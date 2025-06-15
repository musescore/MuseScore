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

import Muse.UiComponents 1.0
import Muse.Ui 1.0
import MuseScore.Inspector 1.0

InspectorPropertyView {
    id: root

    property alias radioButtonGroup: radioButtonGroupItem
    property alias model: radioButtonGroupItem.model

    property int requestHeight: 30
    property int requestWidth: 0
    property int requestIconFontSize: 0

    navigationRowEnd: navigationRowStart /* Menu button */ + radioButtonGroupItem.count /* FlatRadioButtons */

    function focusOnFirst() {
        radioButtonGroupItem.focusOnFirst()
    }

    FlatRadioButtonList {
        id: radioButtonGroupItem

        height: root.requestHeight
        width: requestWidth ? requestWidth : parent.width

        currentValue: root.propertyItem && !root.propertyItem.isUndefined ? root.propertyItem.value : undefined

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
        accessibleName: root.titleText

        iconFontSize: root.requestIconFontSize

        onToggled: function(newValue) {
            if (root.propertyItem) {
                root.propertyItem.value = newValue
            }
        }
    }
}
