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
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0

Row {
    id: root

    property alias title: titleLabel.text
    property alias titleWidth: titleLabel.width

    property alias currentValue: control.currentValue
    property alias minValue: control.minValue
    property alias maxValue: control.maxValue
    property alias measureUnitsSymbol: control.measureUnitsSymbol

    property alias control: control

    signal valueEdited(var newValue)

    spacing: 0

    StyledTextLabel {
        id: titleLabel

        anchors.verticalCenter: parent.verticalCenter

        horizontalAlignment: Qt.AlignLeft
        wrapMode: Text.WordWrap
        maximumLineCount: 2
    }

    IncrementalPropertyControl {
        id: control

        width: 102
        decimals: 0
        step: 1

        onValueEdited: {
            root.valueEdited(newValue)
        }
    }
}
