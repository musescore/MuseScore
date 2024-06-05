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
import QtQuick.Layouts 1.15

import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

RowLayout {
    id: root

    width: parent.width

    required property PropertyItem propertyItem
    property alias label: label
    property alias text: label.text
    property alias navigation: toggle.navigation

    visible: propertyItem && propertyItem.isVisible
    enabled: propertyItem && propertyItem.isEnabled

    spacing: 4

    ToggleButton {
        id: toggle

        checked: root.propertyItem && Boolean(root.propertyItem.value)

        onToggled: {
            if (root.propertyItem) {
                root.propertyItem.value = !checked
            }
        }
    }

    StyledTextLabel {
        id: label

        Layout.fillWidth: true

        horizontalAlignment: Text.AlignLeft
    }
}
