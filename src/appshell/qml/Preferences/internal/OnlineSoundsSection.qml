/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Online sounds")

    rowSpacing: 16

    property alias autoProcessOnlineSoundsInBackground: autoProcessOnlineSoundsInBackground.checked
    property int progressBarMode: 0

    signal autoProcessOnlineSoundsInBackgroundChangeRequested(bool value)
    signal progressBarModeChangeRequired(int mode)


    Column {
        width: parent.width

        spacing: 12

        CheckBox {
            id: autoProcessOnlineSoundsInBackground

            width: parent.width

            text: qsTrc("appshell/preferences", "Automatically process online sounds in the background")

            navigation.name: "AutoProcessOnlineSoundsInBackgroundCheckbox"
            navigation.panel: root.navigation
            navigation.row: 1

            onClicked: {
                root.autoProcessOnlineSoundsInBackgroundChangeRequested(!checked)
            }
        }

        Row {
            spacing: 6

            visible: !autoProcessOnlineSoundsInBackground.checked

            Item {
                width: 20
                height: 20

                StyledIconLabel {
                    anchors.centerIn: parent

                    iconCode: IconCode.INSIGHT
                }
            }

            StyledTextLabel {
                width: showProcessingVisualizationComboBox.x + showProcessingVisualizationComboBox.width - x

                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap
                maximumLineCount: 2

                text: qsTrc("appshell/preferences", "To process online sounds, press ‘Process online sounds’ in the status bar at the bottom of the app window")
            }
        }
    }


    ComboBoxWithTitle {
        id: showProcessingVisualizationComboBox

        control.width: showProcessingVisualizationComboBox.isOpened && root.autoProcessOnlineSoundsInBackground ?
                           378 : showProcessingVisualizationComboBox.columnWidth

        title: qsTrc("appshell/preferences", "Show processing visualization:")

        columnWidth: root.columnWidth
        currentIndex: showProcessingVisualizationComboBox.indexOfValue(root.progressBarMode)

        navigation.name: "ShowProcessingVisualization"
        navigation.panel: root.navigation
        navigation.row: 2

        model: {
            var options = [{ text: qsTrc("appshell/preferences", "Always"), value: 0 }]

            if (root.autoProcessOnlineSoundsInBackground) {
                options.push({ text: qsTrc("appshell/preferences", "Only if processing is unfinished during playback"), value: 1 })
            }

            options.push({ text: qsTrc("appshell/preferences", "Never"), value: 2 })

            return options
        }

        onValueEdited: function(newIndex, newValue) {
            root.progressBarModeChangeRequired(newValue)
        }
    }
}
