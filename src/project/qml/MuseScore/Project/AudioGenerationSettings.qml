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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

RadioButtonGroup {
    id: root

    AudioGenerationSettingsModel {
        id: model
    }

    orientation: Qt.Vertical
    spacing: 16

    model: [
        { text: qsTrc("project/save", "Never"), type: GenerateAudioTimePeriodType.Never },
        { text: qsTrc("project/save", "Always"), type: GenerateAudioTimePeriodType.Always },
        { text: qsTrc("project/save", "Every:"), type: GenerateAudioTimePeriodType.AfterCertainNumberOfSaves },
    ]

    delegate: Loader {
        sourceComponent: modelData["type"] === GenerateAudioTimePeriodType.AfterCertainNumberOfSaves ?
                             numberOfSavesComp : radioBtnComp

        onLoaded: {
            item.text = modelData["text"]
            item.type = modelData["type"]
        }
    }

    Component {
        id: radioBtnComp

        RoundedRadioButton {
            anchors.verticalCenter: parent.verticalCenter

            property int type: 0

            checked: model.timePeriodType === type

            onToggled: {
                model.timePeriodType = type
            }
        }
    }

    Component {
        id: numberOfSavesComp

        Row {
            id: numberOfSavesItem

            spacing: 0

            property string text: ""
            property int type: 0

            RoundedRadioButton {
                anchors.verticalCenter: parent.verticalCenter

                width: 120

                text: numberOfSavesItem.text
                checked: model.timePeriodType === numberOfSavesItem.type

                onToggled: {
                    model.timePeriodType = numberOfSavesItem.type
                }
            }

            IncrementalPropertyControl {
                width: 96

                minValue: 2
                maxValue: 30
                currentValue: model.numberOfSaves
                measureUnitsSymbol: qsTrc("project/save", "Saves")
                step: 1
                decimals: 0

                onValueEdited: function(newValue) {
                    model.numberOfSaves = newValue
                }
            }
        }
    }
}
