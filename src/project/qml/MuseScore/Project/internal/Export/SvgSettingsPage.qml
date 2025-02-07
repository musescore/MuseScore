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

import Muse.UiComponents 1.0

ExportSettingsPage {
    id: root

    CheckBox {
        width: parent.width
        text: qsTrc("project/export", "Transparent background")

        navigation.name: "TransparentBackgroundCheckbox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder + 2

        checked: root.model.svgTransparentBackground
        onClicked: {
            root.model.svgTransparentBackground = !checked
        }
    }

    CheckBox {
        width: parent.width
        text: qsTrc("project/export", "Masking compatibility (Adobe Illustrator only)")

        navigation.name: "IllustratorCompatCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder + 3

        checked: root.model.svgIllustratorCompat
        onClicked: {
            root.model.svgIllustratorCompat = !checked
        }
    }

    StyledTextLabel {
        width: parent.width
        text: qsTrc("project/export", "Each page of the selected parts will be exported as a separate %1 file.").arg("SVG")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
    }
}
