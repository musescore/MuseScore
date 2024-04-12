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

        DropdownPropertyView {
            id: typeSection
            titleText: qsTrc("inspector", "Type")
            propertyItem: root.model ? root.model.ottavaType : null
            model: root.model ? root.model.possibleOttavaTypes() : null

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }

        PropertyCheckBox {
            id: showNumbersOnlyCheckBox
            text: qsTrc("inspector", "Show numbers only")
            propertyItem: root.model ? root.model.showNumbersOnly : null

            navigation.name: "ShowNumbersOnly"
            navigation.panel: root.navigationPanel
            navigation.row: typeSection.navigationRowEnd + 1
        }

        SeparatorLine { anchors.margins: -12 }

        LineWithHooksCommonStyleSettings {
            model: root.model

            navigationPanel: root.navigationPanel
            navigationRowStart: showNumbersOnlyCheckBox.navigation.row + 1
        }
    }
}
