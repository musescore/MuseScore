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

    property var defaultSettingControl: null
    property string defaultSettingControlText: ""

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "AudioGenerationSettingsContent"
        direction: NavigationPanel.Both
    }

    orientation: Qt.Vertical
    spacing: 16

    function focusOnDefaultSettingControl() {
        var item = itemAtIndex(0)
        if (item) {
            root.defaultSettingControl = item
            root.defaultSettingControlText = item.text
            item.activateFocus()
        }
    }

    signal accessibleInfoResetRequested()

    AudioGenerationSettingsModel {
        id: settingsModel
    }

    model: [
        { text: qsTrc("project/save", "Never"), type: GenerateAudioTimePeriodType.Never },
        { text: qsTrc("project/save", "Always"), type: GenerateAudioTimePeriodType.Always },
        { text: qsTrc("project/save", "Every:"), type: GenerateAudioTimePeriodType.AfterCertainNumberOfSaves },
    ]

    delegate: Loader {
        property string text: modelData["text"]

        function activateFocus() {
            item.activateFocus()
        }

        sourceComponent: modelData["type"] === GenerateAudioTimePeriodType.AfterCertainNumberOfSaves ?
                             numberOfSavesComp : radioBtnComp

        onLoaded: {
            item.text = modelData["text"]
            item.type = modelData["type"]
            item.index = model.index
        }
    }

    Component {
        id: radioBtnComp

        RoundedRadioButton {
            anchors.verticalCenter: parent.verticalCenter

            property int type: 0
            property int index: 0

            checked: settingsModel.timePeriodType === type

            navigation.panel: root.navigationPanel
            navigation.row: index
            navigation.column: 0
            navigation.accessible.ignored: this === root.defaultSettingControl
            navigation.onActiveChanged: {
                if (!navigation.active) {
                    navigation.accessible.ignored = false
                    navigation.accessible.focused = true
                    root.accessibleInfoResetRequested()
                }
            }

            function activateFocus() {
                navigation.requestActive()
            }

            onToggled: {
                settingsModel.timePeriodType = type
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
            property int index: 0

            function activateFocus() {
                button.navigation.requestActive()
            }

            RoundedRadioButton {
                id: button

                anchors.verticalCenter: parent.verticalCenter

                width: 120

                text: numberOfSavesItem.text
                checked: settingsModel.timePeriodType === numberOfSavesItem.type

                navigation.panel: root.navigationPanel
                navigation.row: index
                navigation.column: 0

                onToggled: {
                    settingsModel.timePeriodType = numberOfSavesItem.type
                }
            }

            IncrementalPropertyControl {
                width: 96

                minValue: 2
                maxValue: 30
                currentValue: settingsModel.numberOfSaves
                measureUnitsSymbol: qsTrc("project/save", "Saves")
                step: 1
                decimals: 0

                navigation.panel: root.navigationPanel
                navigation.row: index
                navigation.column: 1

                onValueEdited: function(newValue) {
                    settingsModel.numberOfSaves = newValue
                }
            }
        }
    }
}
