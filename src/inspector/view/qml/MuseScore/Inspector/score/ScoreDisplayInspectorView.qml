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
import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import Muse.UiComponents 1.0

import "../common"

InspectorSectionView {
    id: root

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
            text: qsTrc("inspector", "Invisible")
            accessibleText: qsTrc("inspector", "Invisible elements")
            isVisible: model ? model.shouldShowInvisible : false
            onVisibleToggled: { model.shouldShowInvisible = !model.shouldShowInvisible }
        }

        VisibilityBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2

            navigation.panel: root.navigationPanel
            navigation.name: "Formatting"
            navigation.row: root.navigationRow(3)

            text: qsTrc("inspector", "Formatting")
            isVisible: model ? model.shouldShowFormatting : false
            onVisibleToggled: { model.shouldShowFormatting = !model.shouldShowFormatting }
        }

        VisibilityBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2

            navigation.panel: root.navigationPanel
            navigation.name: "Frames"
            navigation.row: root.navigationRow(4)

            text: qsTrc("inspector", "Frames")
            isVisible: model ? model.shouldShowFrames : false
            onVisibleToggled: { model.shouldShowFrames = !model.shouldShowFrames }
        }

        VisibilityBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2

            navigation.panel: root.navigationPanel
            navigation.name: "Page margins"
            navigation.row: root.navigationRow(5)

            text: qsTrc("inspector", "Page margins")
            isVisible: model ? model.shouldShowPageMargins : false
            onVisibleToggled: { model.shouldShowPageMargins = !model.shouldShowPageMargins }
        }

        VisibilityBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2

            navigation.panel: root.navigationPanel
            navigation.name: "Sound flags"
            navigation.row: root.navigationRow(6)

            text: qsTrc("inspector", "Sound flags")
            isVisible: model ? model.shouldShowSoundFlags : false
            onVisibleToggled: { model.shouldShowSoundFlags = !model.shouldShowSoundFlags }
        }
    }
}
