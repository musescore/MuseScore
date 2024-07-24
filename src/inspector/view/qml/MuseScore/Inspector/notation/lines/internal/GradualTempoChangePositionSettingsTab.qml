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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../../common"

FocusableItem {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        PlacementSection {
            id: placementSection

            propertyItem: root.model ? root.model.placement : null

            navigationPanel: root.navigationPanel
            navigationRowStart: lineStyleSection.navigationRowEnd + 1
        }

        SeparatorLine {
            anchors.margins: -12
        }

        StyledTextLabel {
            text: qsTrc("inspector", "Alignment with adjacent tempo text")
        }

        PropertyCheckBox {
            id: snapAfter
            text: qsTrc("inspector", "Snap to next")
            propertyItem: root.model ? root.model.snapAfter : null

            navigation.panel: root.navigationPanel
            navigation.row: snapBefore.navigation.row + 1
        }
    }
}
