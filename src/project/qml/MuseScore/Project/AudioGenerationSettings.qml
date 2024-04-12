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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
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
        { text: "" /*see numberOfSavesComp*/, type: GenerateAudioTimePeriodType.AfterCertainNumberOfSaves },
    ]

    delegate: Loader {
        property string text: modelData["text"]

        function activateFocus() {
            item.activateFocus()
        }

        width: parent.width

        sourceComponent: modelData["type"] === GenerateAudioTimePeriodType.AfterCertainNumberOfSaves
                         ? numberOfSavesComp : radioBtnComp

        onLoaded: {
            if (modelData["type"] !== GenerateAudioTimePeriodType.AfterCertainNumberOfSaves) {
                item.text = modelData["text"]
            }

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

            width: parent.width

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

        RowLayout {
            id: numberOfSavesItem

            width: parent.width
            spacing: 6

            // "Every: %1 saves" needs to be one string for correct translatability. We then split the translated version.

            //: `%1` will be replaced with a number input field.
            //: Text before it will appear before that number field, text after will appear after the field.
            readonly property string text: qsTrc("project/save", "Every: %1 saves")

            readonly property var textSplit: text.split("%1")

            readonly property string textPart1: textSplit[0]
            readonly property string textPart2: textSplit[1]

            // If the translation of "saves" is short, put it inside the spinbox, otherwise in a separate label outside.
            readonly property int textPart2InSpinboxThreshold: 6
            readonly property bool textPart2InSpinbox: textPart2.length <= textPart2InSpinboxThreshold

            property int type: 0
            property int index: 0

            function activateFocus() {
                button.navigation.requestActive()
            }

            RoundedRadioButton {
                id: button

                Layout.minimumWidth: 80

                text: numberOfSavesItem.textPart1.trim()
                checked: settingsModel.timePeriodType === numberOfSavesItem.type

                navigation.panel: root.navigationPanel
                navigation.row: numberOfSavesItem.index
                navigation.column: 0

                //: Accessibility name for "Every N saves" radio button in MP3 generation settings dialog
                navigation.accessible.name: qsTrc("project/save", "Every N saves")

                onToggled: {
                    settingsModel.timePeriodType = numberOfSavesItem.type
                }
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: numberOfSavesItem.textPart2InSpinbox ? 96 : 60

                minValue: 2
                maxValue: 30
                currentValue: settingsModel.numberOfSaves
                measureUnitsSymbol: numberOfSavesItem.textPart2InSpinbox ? numberOfSavesItem.textPart2 : ""
                step: 1
                decimals: 0

                navigation.panel: root.navigationPanel
                navigation.row: numberOfSavesItem.index
                navigation.column: 1
                navigation.accessible.name: qsTrc("project/save", "Every N saves")

                onValueEdited: function(newValue) {
                    settingsModel.numberOfSaves = newValue
                }
            }

            StyledTextLabel {
                Layout.fillWidth: true

                text: numberOfSavesItem.textPart2InSpinbox ? "" : numberOfSavesItem.textPart2.trim()
                horizontalAlignment: Text.AlignLeft
            }
        }
    }
}
