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

InspectorPropertyView {
    id: root

    visible: Boolean(propertyItem)

    navigationName: "TextSection"

    function focusOnFirst() {
        textField.navigation.requestActive()
    }

    TextInputField {
        id: textField
        isIndeterminate: root.propertyItem ? root.propertyItem.isUndefined : false
        currentText: root.propertyItem ? root.propertyItem.value : ""
        enabled: root.propertyItem ? root.propertyItem.isEnabled : false

        navigation.name: root.navigationName + " TextInputField"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1
        navigation.accessible.name: root.titleText + " " + currentText

        onTextEditingFinished: function(newTextValue) {
            if (root.propertyItem) {
                root.propertyItem.value = newTextValue
            }
        }
    }
}
