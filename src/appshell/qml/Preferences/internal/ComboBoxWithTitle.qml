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

    property alias currentIndex: comboBox.currentIndex
    property alias currentValue: comboBox.currentValue
    property alias model: comboBox.model
    property alias control: comboBox

    signal valueEdited(var newValue)

    spacing: 0

    StyledTextLabel {
        id: titleLabel

        anchors.verticalCenter: parent.verticalCenter

        width: root.firstColumnWidth
        horizontalAlignment: Qt.AlignLeft
    }

   Dropdown {
        id: comboBox

        width: 210

        onCurrentValueChanged: {
            root.valueEdited(comboBox.currentValue)
        }
    }
}
