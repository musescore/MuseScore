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

import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    enabled: model ? !model.isEmpty : false
    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: Column {
        width: parent.width

        spacing: 8

        SpinBoxPropertyView {
            navigationName: "Amount"
            navigationPanel: root.navigation.panel
            navigationRowStart: root.navigation.row + 1

            titleText: qsTrc("inspector", "Amount")
            propertyItem: root.model ? root.model.tempoChangeFactor : null

            minValue: 1
            maxValue: 1000
            decimals: 0
            step: 1
            measureUnitsSymbol: "%"
        }

        DropdownPropertyView {
            navigationName: "Easing method"
            navigationPanel: root.navigation.panel
            navigationRowStart: root.navigation.row + 2

            titleText: qsTrc("inspector", "Easing method")
            propertyItem: root.model ? root.model.tempoEasingMethod : null

            model: root.model ? root.model.possibleEasingMethods() : []
        }
    }
}
