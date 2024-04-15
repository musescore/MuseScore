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
import MuseScore.Inspector 1.0

InspectorPropertyView {
    id: root

    height: implicitHeight
    width: parent.width

    titleText: qsTrc("inspector", "Color")

    navigationName: "Color Section"
    navigationRowEnd: colorPicker.navigation.row

    ColorPicker {
        id: colorPicker

        navigation.name: root.navigationName + " ColorPicker"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1
        navigation.accessible.name: root.titleText + " " + Utils.accessibleColorDescription(colorPicker.color)

        enabled: root.propertyItem ? root.propertyItem.isEnabled : false
        isIndeterminate: root.propertyItem && enabled ? root.propertyItem.isUndefined : false
        color: root.propertyItem && !root.propertyItem.isUndefined ? root.propertyItem.value : ui.theme.backgroundPrimaryColor

        onNewColorSelected: function(newColor) {
            if (root.propertyItem) {
                root.propertyItem.value = newColor
            }
        }
    }
}
