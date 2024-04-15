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

import Muse.UiComponents 1.0
import MuseScore.Project 1.0

ExportSettingsPage {
    id: root

    ExportOptionItem {
        id: resolutionLabel
        text: qsTrc("project/export", "Resolution:")

        IncrementalPropertyControl {
            Layout.preferredWidth: 80

            navigation.name: "ResolutionSpinbox"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationOrder + 1
            navigation.accessible.name: resolutionLabel.text + " " + String(currentValue)

            currentValue: root.model.pngResolution
            minValue: 32
            maxValue: 5000
            step: 1
            decimals: 0

            //: Dots per inch
            measureUnitsSymbol: qsTrc("global", "dpi")

            onValueEdited: function(newValue) {
                root.model.pngResolution = newValue
            }
        }
    }

    CheckBox {
        width: parent.width
        text: qsTrc("project/export", "Transparent background")

        navigation.name: "TransparentBackgroundCheckbox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder + 2

        checked: root.model.pngTransparentBackground
        onClicked: {
            root.model.pngTransparentBackground = !checked
        }
    }

    StyledTextLabel {
        width: parent.width
        text: qsTrc("project/export", "Each page of the selected parts will be exported as a separate %1 file.").arg("PNG")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
    }
}
