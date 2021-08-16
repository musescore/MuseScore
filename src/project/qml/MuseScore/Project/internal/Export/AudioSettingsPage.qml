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
import QtQuick 2
import QtQuick.Layouts 1

import MuseScore.UiComponents 1
import MuseScore.Project 1

ColumnLayout {
    id: root
    spacing: 12

    property ExportDialogModel model
    property int firstColumnWidth

    property bool showBitRateControl: false

    CheckBox {
        text: qsTrc("project", "Normalize")
        checked: root.model.normalizeAudio
        onClicked: {
            root.model.normalizeAudio = !checked
        }
    }

    ExportOptionItem {
        Layout.fillWidth: false
        text: qsTrc("project", "Sample rate:")
        firstColumnWidth: root.firstColumnWidth

        Dropdown {
            id: srates
            Layout.preferredWidth: 126

            model: root.model.availableSampleRates().map(function (sampleRate) {
                return { text: qsTrc("project", "%1 Hz").arg(sampleRate), value: sampleRate }
            })

            currentIndex: srates.indexOfValue(root.model.sampleRate)
            onCurrentValueChanged: {
                root.model.sampleRate = srates.currentValue
            }
        }
    }

    ExportOptionItem {
        visible: root.showBitRateControl
        text: qsTrc("project", "Bitrate:")
        firstColumnWidth: root.firstColumnWidth

        Dropdown {
            id: bitrates
            Layout.preferredWidth: 126

            model: root.model.availableBitRates().map(function (bitRate) {
                return { text: qsTrc("project", "%1 kBit/s").arg(bitRate), value: bitRate }
            })

            currentIndex: bitrates.indexOfValue(root.model.bitRate)
            onCurrentValueChanged: {
                root.model.bitRate = bitrates.currentValue
            }
        }
    }

    StyledTextLabel {
        Layout.fillWidth: true
        text: qsTrc("project", "Each selected part will be exported as a separate audio file.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
    }
}
