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
import "internal"

Column {
    id: root

    property QtObject model: null

    objectName: "HorizontalFrameSettings"

    height: implicitHeight
    spacing: 16

    WidthSection {
        widthProperty: root.model ? root.model.frameWidth : null
    }

    SeparatorLine { anchors.margins: -10 }

    HorizontalGapsSection {
        leftGap: root.model ? root.model.leftGap : null
        rightGap: root.model ? root.model.rightGap: null
    }

    SeparatorLine { anchors.margins: -10 }

    CheckBox {
        isIndeterminate: root.model ? root.model.shouldDisplayKeysAndBrackets.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.shouldDisplayKeysAndBrackets.value : false
        text: qsTrc("inspector", "Display key, brackets and braces")

        onClicked: { root.model.shouldDisplayKeysAndBrackets.value = !checked }
    }
}
