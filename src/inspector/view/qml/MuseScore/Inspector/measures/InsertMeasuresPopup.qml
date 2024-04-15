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
import Muse.Ui 1.0
import MuseScore.Inspector 1.0

import "../common"

Column {
    id: root

    property MeasuresSettingsModel model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0

    width: parent.width

    spacing: 12

    function forceFocusIn() {
        numberOfMeasures.navigation.requestActive()
    }

    RowLayout {
        width: parent.width
        spacing: 4

        IncrementalPropertyControl {
            id: numberOfMeasures
            Layout.preferredWidth: 60

            navigation.name: "NumberOfMeasuresSpinBox"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart
            navigation.accessible.name: qsTrc("inspector", "Number of measures to insert:") + " " + currentValue

            currentValue: 1
            decimals: 0
            step: 1
            minValue: 1
            maxValue: 999

            onValueEdited: function(newValue) {
                if (currentValue !== newValue) {
                    currentValue = newValue
                }
            }
        }

        StyledDropdown {
            id: targetDropdown
            Layout.fillWidth: true

            navigation.name: "TargetDropdown"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
            navigation.accessible.name: qsTrc("inspector", "Where to insert measures:") + " " + currentText

            model: [
                { text: qsTrc("notation", "After selection"), value: MeasuresSettingsModel.AfterSelection },
                { text: qsTrc("notation", "Before selection"), value: MeasuresSettingsModel.BeforeSelection },
                { text: qsTrc("notation", "At start of score"), value: MeasuresSettingsModel.AtStartOfScore },
                { text: qsTrc("notation", "At end of score"), value: MeasuresSettingsModel.AtEndOfScore }
            ]

            currentIndex: 0

            onActivated: function(index, value) {
                currentIndex = index
            }
        }
    }

    SeparatorLine {
        anchors.margins: -12
    }

    FlatButton {
        id: doInsertButton
        width: parent.width

        navigation.name: "DoInsertButton"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 2
        navigation.accessible.name: qsTrc("inspector", "Insert measures")

        icon: IconCode.PLUS

        onClicked: {
            model.insertMeasures(numberOfMeasures.currentValue, targetDropdown.currentValue)
        }
    }
}
