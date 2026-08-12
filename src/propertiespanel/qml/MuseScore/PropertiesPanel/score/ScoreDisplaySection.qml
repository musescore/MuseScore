/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
import MuseScore.PropertiesPanel

import "../common"

PropertiesPanelSection {
    id: root

    required property ScoreDisplaySettingsModel model

    implicitHeight: grid.implicitHeight

    GridLayout {
        id: grid

        width: parent.width

        columns: 2

        rowSpacing: 12
        columnSpacing: 4

        VisibilityBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width / 2

            navigation.panel: root.navigationPanel
            navigation.name: "Invisible"
            navigation.row: root.navigationRow(2)

            //: Invisible elements
            text: qsTrc("propertiespanel", "Invisible")
            accessibleText: qsTrc("propertiespanel", "Invisible elements")
            isVisible: root.model ? root.model.shouldShowInvisible : false
            onVisibleToggled: { root.model.shouldShowInvisible = !root.model.shouldShowInvisible }
        }

        VisibilityBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2

            navigation.panel: root.navigationPanel
            navigation.name: "Formatting"
            navigation.row: root.navigationRow(3)

            text: qsTrc("propertiespanel", "Formatting")
            isVisible: root.model ? root.model.shouldShowFormatting : false
            onVisibleToggled: { root.model.shouldShowFormatting = !root.model.shouldShowFormatting }
        }

        VisibilityBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2

            navigation.panel: root.navigationPanel
            navigation.name: "Frames"
            navigation.row: root.navigationRow(4)

            text: qsTrc("propertiespanel", "Frames")
            isVisible: root.model ? root.model.shouldShowFrames : false
            onVisibleToggled: { root.model.shouldShowFrames = !root.model.shouldShowFrames }
        }

        VisibilityBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2

            navigation.panel: root.navigationPanel
            navigation.name: "Page margins"
            navigation.row: root.navigationRow(5)

            text: qsTrc("propertiespanel", "Page margins")
            isVisible: root.model ? root.model.shouldShowPageMargins : false
            onVisibleToggled: { root.model.shouldShowPageMargins = !root.model.shouldShowPageMargins }
        }

        VisibilityBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2

            navigation.panel: root.navigationPanel
            navigation.name: "Sound flags"
            navigation.row: root.navigationRow(6)

            text: qsTrc("propertiespanel", "Sound flags")
            isVisible: root.model ? root.model.shouldShowSoundFlags : false
            onVisibleToggled: { root.model.shouldShowSoundFlags = !root.model.shouldShowSoundFlags }
        }
    }
}
