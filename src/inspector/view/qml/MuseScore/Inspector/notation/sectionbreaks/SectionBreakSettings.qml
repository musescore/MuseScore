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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "SectionBreakSettings"

    spacing: 12

    function focusOnFirst() {
        pauseBeforeStartsSection.focusOnFirst()
    }

    SpinBoxPropertyView {
        id: pauseBeforeStartsSection
        titleText: qsTrc("inspector", "Pause before new section starts")
        propertyItem: root.model ? root.model.pauseDuration : null

        maxValue: 999
        minValue: 0
        step: 0.5

        //: Abbreviation of "seconds"
        measureUnitsSymbol: qsTrc("global", "s")

        navigationName: "PauseBeforeStarts"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
    }

    PropertyCheckBox {
        id: startWithLongInstrNames
        text: qsTrc("inspector", "Start new section with long instrument names")
        propertyItem: root.model ? root.model.shouldStartWithLongInstrNames : null

        navigation.name: "StartWithLong"
        navigation.panel: root.navigationPanel
        navigation.row: pauseBeforeStartsSection.navigationRowEnd + 1
    }

    PropertyCheckBox {
        id: shouldResetBarNums
        text: qsTrc("inspector", "Reset measure numbers for new section")
        propertyItem: root.model ? root.model.shouldResetBarNums : null

        navigation.name: "ResetBarNumbers"
        navigation.panel: root.navigationPanel
        navigation.row: startWithLongInstrNames.navigation.row + 1
    }

    PropertyCheckBox {
        id: startWithfirstSystemIndented
        text: qsTrc("inspector", "Indent first system of new section")
        propertyItem: root.model ? root.model.firstSystemIndent : null

        navigation.name: "FirstSystemIndent"
        navigation.panel: root.navigationPanel
        navigation.row: shouldResetBarNums.navigation.row + 1
    }

    PropertyCheckBox {
        id: showCourtesySignatures
        text: qsTrc("inspector", "Hide courtesy clefs and signatures")
        propertyItem: root.model ? root.model.showCourtesySignatures : null

        checked: propertyItem && !Boolean(propertyItem.value)
        onClicked: {
            if (propertyItem) {
                propertyItem.value = checked
            }
        }

        navigation.name: "ShowCourtesySignatures"
        navigation.panel: root.navigationPanel
        navigation.row: startWithfirstSystemIndented.navigation.row + 1
    }
}
