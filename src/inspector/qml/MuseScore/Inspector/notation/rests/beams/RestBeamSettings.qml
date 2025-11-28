/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

import MuseScore.Inspector
import Muse.UiComponents
import Muse.Ui

import "../beams"

FocusableItem {
    id: root

    property RestBeamSettingsModel model: null
    readonly property BeamModesModel beamModesModel: model?.beamModesModel ?? null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1
    readonly property int navigationRowEnd: beamTypeSection.navigationRowEnd

    implicitWidth: beamTypeSection.implicitWidth
    implicitHeight: beamTypeSection.implicitHeight

    function focusOnFirst() {
        beamTypeSection.focusOnFirst()
    }

    BeamTypeSelector {
        id: beamTypeSection
        titleText: qsTrc("inspector", "Beam type")
        propertyItem: root.beamModesModel ? root.beamModesModel.mode : null
        enabled: root.beamModesModel && !root.beamModesModel.isEmpty

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
    }
}
