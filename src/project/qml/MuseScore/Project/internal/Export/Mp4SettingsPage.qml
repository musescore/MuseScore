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
import QtQuick
import QtQuick.Layouts

import Muse.UiComponents
import Muse.Ui
import MuseScore.Project

ExportSettingsPage {
    id: root

    AudioSettings {
        model: root.model
        navigationPanel: root.navigationPanel
        navigationOrder: root.navigationOrder
        showBitRateControl: true
    }

    ExportOptionItem {
        id: videoResolutionLabel
        text: qsTrc("project/export", "Video resolution:")

        StyledDropdown {
            Layout.preferredWidth: 126

            navigation.name: "VideoResolutionDropdown"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationOrder + 10
            navigation.accessible.name: videoResolutionLabel.text + " " + currentText

            model: root.model ? root.model.availableVideoResolutions().map(function(res) {
                return { text: res, value: res }
            }) : []

            currentIndex: root.model ? indexOfValue(root.model.videoResolution) : -1

            onActivated: function(index, value) {
                root.model.videoResolution = value
            }
        }
    }

    StyledTextLabel {
        width: parent.width
        text: qsTrc("project/export", "Each selected part will be exported as a separate video file.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
    }
}
