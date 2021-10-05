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
import MuseScore.Project 1.0

ExportSettingsPage {
    id: root

    ExportOptionItem {
        id: resolutionLabel
        text: qsTrc("project", "Resolution:")

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
            measureUnitsSymbol: qsTrc("project", "dpi")

            onValueEdited: function(newValue) {
                root.model.pngResolution = newValue
            }
        }
    }

    CheckBox {
        width: parent.width
        text: qsTrc("project", "Transparent background")

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
        text: qsTrc("project", "Each page of the selected parts will be exported as a separate PNG file.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
    }
}
