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
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Project 1.0

ExportSettingsPage {
    id: root

    property bool showBitRateControl: false

    CheckBox {
        width: parent.width
        text: qsTrc("project", "Normalize")

        navigation.name: "NormalizeAudioCheckbox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder + 1

        checked: root.model.normalizeAudio
        onClicked: {
            root.model.normalizeAudio = !checked
        }
    }

    ExportOptionItem {
        id: sampleRateLabel
        text: qsTrc("project", "Sample rate:")

        Dropdown {
            Layout.preferredWidth: 126

            navigation.name: "SampleRatesDropdown"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationOrder + 2
            navigation.accessible.name: sampleRateLabel.text + " " + currentText

            model: root.model.availableSampleRates().map(function(sampleRate) {
                return { text: qsTrc("project", "%1 Hz").arg(sampleRate), value: sampleRate }
            })

            currentIndex: indexOfValue(root.model.sampleRate)
            onCurrentValueChanged: {
                root.model.sampleRate = currentValue
            }
        }
    }

    ExportOptionItem {
        id: bitrateLabel
        visible: root.showBitRateControl
        text: qsTrc("project", "Bitrate:")

        Dropdown {
            Layout.preferredWidth: 126

            navigation.name: "BitratesDropdown"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationOrder + 3
            navigation.accessible.name: bitrateLabel.text + " " + currentText

            model: root.model.availableBitRates().map(function(bitRate) {
                return { text: qsTrc("project", "%1 kBit/s").arg(bitRate), value: bitRate }
            })

            currentIndex: indexOfValue(root.model.bitRate)
            onCurrentValueChanged: {
                root.model.bitRate = currentValue
            }
        }
    }

    StyledTextLabel {
        width: parent.width
        text: qsTrc("project", "Each selected part will be exported as a separate audio file.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
    }
}
