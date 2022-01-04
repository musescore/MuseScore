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

import MuseScore.UiComponents 1.0

BaseSection {
    id: root

    property alias currentInputDeviceIndex: inputDevicesBox.currentIndex
    property alias currentOutputDeviceIndex: outputDevicesBox.currentIndex

    property alias inputDevices: inputDevicesBox.model
    property alias outputDevices: outputDevicesBox.model

    signal currentInputDeviceIndexChangeRequested(int newIndex)
    signal currentOuputDeviceIndexChangeRequested(int newIndex)

    title: qsTrc("appshell", "MIDI")

    ComboBoxWithTitle {
        id: inputDevicesBox

        title: qsTrc("appshell", "MIDI input:")
        columnWidth: root.columnWidth

        navigation.name: "MidiInputBox"
        navigation.panel: root.navigation
        navigation.row: 1

        onValueEdited: function(newValue) {
            root.currentInputDeviceIndexChangeRequested(currentIndex)
        }
    }

    ComboBoxWithTitle {
        id: outputDevicesBox

        title: qsTrc("appshell", "MIDI output:")
        columnWidth: root.columnWidth

        navigation.name: "MidiOutputBox"
        navigation.panel: root.navigation
        navigation.row: 2

        onValueEdited: function(newValue) {
            root.currentOuputDeviceIndexChangeRequested(currentIndex)
        }
    }
}
