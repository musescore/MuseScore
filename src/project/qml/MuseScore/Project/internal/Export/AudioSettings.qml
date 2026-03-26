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
import QtQuick
import QtQuick.Layouts

import Muse.UiComponents
import Muse.Ui
import MuseScore.Project

Row {
    id: root

    property ExportDialogModel model
    property NavigationPanel navigationPanel: null
    property int navigationOrderStart: 0
    property alias navigationOrderEnd: sampleFormatsDropdown.navigation.row

    property bool showBitRateControl: false
    property bool showSampleRateControl: true

    width: parent ? parent.width : implicitWidth

    spacing: 12

    ExportOptionItem {
        id: sampleRateLabel
        visible: root.showSampleRateControl
        text: qsTrc("project/export", "Sample rate:")

        StyledDropdown {
            Layout.preferredWidth: 126

            navigation.name: "SampleRatesDropdown"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationOrderStart + 1
            navigation.accessible.name: sampleRateLabel.text + " " + currentText

            model: root.model ? root.model.availableSampleRates().map(function(sampleRate) {
                return { text: qsTrc("project/export", "%1 Hz").arg(sampleRate), value: sampleRate }
            }) : []

            currentIndex: root.model ? indexOfValue(root.model.sampleRate) : -1

            onActivated: function(index, value) {
                root.model.sampleRate = value
            }
        }
    }

    ExportOptionItem {
        id: bitrateLabel
        visible: root.showBitRateControl
        text: qsTrc("project/export", "Bitrate:")

        StyledDropdown {
            Layout.preferredWidth: 126

            navigation.name: "BitratesDropdown"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationOrderStart + 2
            navigation.accessible.name: bitrateLabel.text + " " + currentText

            model: root.model ? root.model.availableBitRates().map(function(bitRate) {
                return { text: qsTrc("project/export", "%1 kBit/s").arg(bitRate), value: bitRate }
            }) : []

            currentIndex: root.model ? indexOfValue(root.model.bitRate) : -1

            onActivated: function(index, value) {
                root.model.bitRate = value
            }
        }
    }

    ExportOptionItem {
        id: sampleFormatLabel
        visible: root.model ? root.model.availableSampleFormats.length > 0 : false
        text: qsTrc("project/export", "Sample format:")

        StyledDropdown {
            id: sampleFormatsDropdown

            Layout.preferredWidth: 126

            navigation.name: "SampleFormatsDropdown"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationOrderStart + 3
            navigation.accessible.name: sampleFormatLabel.text + " " + currentText

            model: root.model ? root.model.availableSampleFormats : []

            currentIndex: root.model ? indexOfValue(root.model.selectedSampleFormat) : -1

            onActivated: function(index, value) {
                root.model.selectedSampleFormat = value
            }
        }
    }
}
