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
import QtQuick.Layouts 1.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

RowLayout {
    id: root
    required property StyleItem styleItem
    required property string label
    property double labelAreaWidth: 120
    property double controlAreaWidth: 326

    property double labelWidth: labelAreaWidth
    property double controlWidth: controlAreaWidth

    default property alias data: control.data

    property bool hasReset: true

    height: control.height

    spacing: 8

    StyledTextLabel {
        id: optionLabel
        horizontalAlignment: Text.AlignLeft
        Layout.fillWidth: true
        Layout.maximumWidth: Math.min(root.labelWidth, root.labelAreaWidth)
        wrapMode: Text.WordWrap
        text: label
    }

    Item {
        id: control
        Layout.preferredWidth: Math.min(root.controlWidth, root.controlAreaWidth)
        Layout.alignment: Qt.AlignTop
    }

    Item {
        Layout.preferredWidth: Math.max(root.controlAreaWidth - control.width - root.spacing - 1, 0)
    }

    FlatButton {
        id: resetButton
        visible: hasReset
        icon: IconCode.UNDO
        enabled: !styleItem.isDefault
        onClicked: styleItem.value = styleItem.defaultValue
    }
}
