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
import QtQuick 2.9

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Row {
    id: root

    property int numerator: 0
    property int denominator: 0
    property var availableDenominators: null

    signal numeratorSelected(var value)
    signal denominatorSelected(var value)

    property NavigationSection navigationSection: null
    property int navigationPanelOrderStart: 0
    property int navigationPanelOrderEnd: timeComboBox.navigation.panel.order

    spacing: 12

    IncrementalPropertyControl {
        id: control

        implicitWidth: 60
        anchors.verticalCenter: parent.verticalCenter

        currentValue: root.numerator
        step: 1
        decimals: 0
        maxValue: 63
        minValue: 1

        navigation.name: "TimeControl"
        navigation.panel: NavigationPanel {
            name: "TimeSignatureNumeratorPanel"
            section: root.navigationSection
            order: root.navigationPanelOrderStart
        }
        navigation.row: 0

        onValueEdited: function(newValue) {
            root.numeratorSelected(newValue)
        }
    }

    StyledTextLabel {
        width: 8
        anchors.verticalCenter: parent.verticalCenter
        font: ui.theme.largeBodyFont
        text: "/"
    }

    StyledDropdown {
        id: timeComboBox

        width: control.width
        anchors.verticalCenter: parent.verticalCenter

        popupItemsCount: 4

        navigation.name: "TimeBox"
        navigation.panel: NavigationPanel {
            name: "TimeSignatureDenominatorPanel"
            section: root.navigationSection
            order: root.navigationPanelOrderStart + 1
        }
        navigation.row: 0

        currentIndex: timeComboBox.indexOfValue(root.denominator)

        model: {
            var resultList = []

            var denominators = root.availableDenominators
            for (var i = 0; i < denominators.length; ++i) {
                resultList.push({"text" : denominators[i], "value" : denominators[i]})
            }

            return resultList
        }

        onActivated: function(index, value) {
            root.denominatorSelected(value)
        }
    }
}
