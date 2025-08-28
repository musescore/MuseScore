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

    property alias showErrorDialog: showErrorDialogBox.checked
    property alias autoProcessOnlineSoundsInBackground: autoProcessOnlineSoundsInBackgroundBox.checked
    property int progressBarMode: 0

    signal showErrorDialogChangeRequested(bool value)
    signal autoProcessOnlineSoundsInBackgroundChangeRequested(bool value)
    signal progressBarModeChangeRequired(int mode)


    Column {
        width: parent.width

        spacing: 12

        CheckBox {
            id: showErrorDialogBox

            width: parent.width

            text: qsTrc("appshell/preferences", "Show error dialog when online sounds cannot be processed")

            navigation.name: "ShowErrorDialogBox"
            navigation.panel: root.navigation
            navigation.row: 1

            onClicked: {
                root.showErrorDialogChangeRequested(!checked)
            }
        }

        CheckBox {
            id: autoProcessOnlineSoundsInBackgroundBox

            width: parent.width

            text: qsTrc("appshell/preferences", "Automatically process online sounds in the background")

            navigation.name: "AutoProcessOnlineSoundsInBackgroundBox"
            navigation.panel: root.navigation
            navigation.row: 2

            onClicked: {
                root.autoProcessOnlineSoundsInBackgroundChangeRequested(!checked)
            }
        }

        Row {
            spacing: 6

            visible: !autoProcessOnlineSoundsInBackgroundBox.checked

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

        title: qsTrc("appshell/preferences", "Show processing visualization")

        columnWidth: root.columnWidth
        currentIndex: showProcessingVisualizationComboBox.indexOfValue(root.progressBarMode)

        navigation.name: "ShowProcessingVisualization"
        navigation.panel: root.navigation
        navigation.row: 3

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
