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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0

InspectorPropertyView {
    id: root

    property alias radioButtonGroup: radioButtonGroupItem
    property alias model: radioButtonGroupItem.model
    property int requestHeight: 30
    property int requestIconFontSize: 0

    navigationRowEnd: navigationRowStart /* Menu button */ + radioButtonGroupItem.count /* FlatRadioButtons */

    function focusOnFirst() {
        radioButtonGroupItem.itemAtIndex(0).navigation.requestActive()
    }

    component Delegate: FlatRadioButton {
        required property var modelData
        required property int index

        text: modelData["text"] ?? ""
        iconCode: modelData["iconCode"] ?? IconCode.NONE
        iconFontSize: root.requestIconFontSize != 0 ? root.requestIconFontSize : ui.theme.iconsFont.pixelSize

        navigation.name: root.navigationName + (Boolean(text) ? text : modelData["title"])
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1 + index
        navigation.accessible.name: root.titleText + " " + (Boolean(text) ? text : modelData["title"])

        checked: root.propertyItem && !root.propertyItem.isUndefined
                 ? root.propertyItem.value === modelData["value"]
                 : false
        onToggled: {
            if (root.propertyItem) {
                root.propertyItem.value = modelData["value"]
            }
        }
    }

    RadioButtonGroup {
        id: radioButtonGroupItem

        height: root.requestHeight
        width: parent.width

        delegate: Delegate {}
    }
}
