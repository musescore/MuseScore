/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
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

import Muse.UiComponents
import Muse.Ui

RadioButtonGroup {
    id: root

    property var currentValue: undefined

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0
    readonly property int navigationRowEnd: root.navigationRowStart + root.count
    property string accessibleName: ""
    property bool transparent: false

    property int iconFontSize: 0

    signal toggled(var newValue)

    function focusOnFirst() {
        if (root.count === 0) {
            return
        }

        (root.itemAtIndex(0) as FlatRadioButton).navigation.requestActive()
    }

    implicitHeight: ui.theme.defaultButtonSize

    delegate: FlatRadioButton {
        required property var modelData
        required property int index

        checked: root.currentValue  === modelData.value

        text: modelData.text ?? ""
        iconCode: modelData.iconCode ?? IconCode.NONE
        iconFontSize: root.iconFontSize != 0 ? root.iconFontSize : ui.theme.iconsFont.pixelSize

        transparent: root.transparent

        toolTipTitle: modelData.title ?? ""
        toolTipDescription: modelData.description ?? ""

        navigation.name: "FlatRadioButtonList_" + (Boolean(text) ? text : modelData.title)
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1 + index
        navigation.accessible.name: root.accessibleName + " " + (Boolean(text) ? text : modelData.title)
        navigation.accessible.description:  modelData.description ?? ""

        onToggled: {
            root.toggled(modelData.value)
        }
    }
}
