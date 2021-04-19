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
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Item {
    id: root

    property int firstColumnWidth: 0

    width: parent.width
    height: content.height

    CommonAudioApiConfigurationModel {
        id: apiModel
    }

    Column {
        id: content

        spacing: 12

        ComboBoxWithTitle {
            title: qsTrc("appshell", "Audio device:")
            titleWidth: root.firstColumnWidth

            currentIndex: apiModel.currentDeviceIndex
            model: apiModel.deviceList()

            onValueEdited: {
                apiModel.currentDeviceIndex = currentIndex
            }
        }

        ComboBoxWithTitle {
            title: qsTrc("appshell", "Sample rate:")
            titleWidth: root.firstColumnWidth

            currentIndex: apiModel.currentSampleRateIndex
            model: apiModel.sampleRateHzList()
            control.displayText: currentValue

            onValueEdited: {
                apiModel.currentSampleRateIndex = currentIndex
            }
        }
    }
}
