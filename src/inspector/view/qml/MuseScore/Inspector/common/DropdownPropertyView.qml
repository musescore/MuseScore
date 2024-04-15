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
import QtQuick.Controls 2.15

import Muse.UiComponents 1.0
import Muse.Ui 1.0
import MuseScore.Inspector 1.0

InspectorPropertyView {
    id: root

    property alias dropdown: dropdownItem
    property alias model: dropdownItem.model

    navigationName: "DropdownPropertyView"
    navigationRowEnd: dropdownItem.navigation.row

    function focusOnFirst() {
        dropdownItem.navigation.requestActive()
    }

    StyledDropdown {
        id: dropdownItem

        width: parent.width

        navigation.name: root.navigationName + " Dropdown"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1
        navigation.accessible.name: root.titleText + " " + currentText

        currentIndex: root.propertyItem && !root.propertyItem.isUndefined
                      ? dropdownItem.indexOfValue(root.propertyItem.value)
                      : -1

        onActivated: function(index, value) {
            root.propertyItem.value = value
        }
    }
}
